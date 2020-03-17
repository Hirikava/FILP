#ifndef DUMMY_PAGE_ALLOCATOR_INCLUDED
#define DUMMY_PAGE_ALLOCATOR_INCLUDED

namespace allocators
{
	template <typename T>
	class DummyPageAllocator
	{
	public:
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T value_type;

		DummyPageAllocator() //dummy dummy stateholder
		{
			head = new Page(1048576);
			current = head;
		}

		~DummyPageAllocator()
		{
			delete head;
		}

		pointer allocate(size_t n, const void* hint = 0)
		{

			if (current->page_size - current->page_offset >= entity_size * n)
			{
				pointer ret_pointer = reinterpret_cast<pointer>(current->page_pointer + current->page_offset);
				current->page_offset += entity_size * n;
				return ret_pointer;
			}
			current->next = new Page(1048576);
			current = current->next;
			return this->allocate(n, hint);
		}

		void deallocate(pointer ptr, size_t n)
		{}
	private:
		struct Page
		{
			char* page_pointer;
			size_t page_size;
			int page_offset;
			Page* next;

			Page(size_t size) : page_size(size), page_offset(0)
			{
				page_pointer = new char[size];
				next = nullptr;
			}

			~Page()
			{
				delete[] page_pointer;
				delete next;
			}

		};

		size_t entity_size = sizeof(T);
		Page* head;
		Page* current;
	};
}


#endif
