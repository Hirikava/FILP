#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>


class CStyleStringComparator
{
public:
	bool operator()(const char* word1, const char* word2) const
	{
		if (word1 == word2)
			return false;

		while (*word1 != '\0' && *word2 != '\0')
		{
			if (*word1 != *word2)
				return *word1 < *word2;
			word1++;
			word2++;
		}

		return *word1 < *word2;
	}
};

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

		if(current->page_size - current->page_offset >= entity_size*n)
		{
			pointer ret_pointer = reinterpret_cast<pointer>(current->page_pointer + current->page_offset);
			current->page_offset += entity_size * n;
			return ret_pointer;
		}
		else
		{
			current->next = new Page(1048576);
			current = current->next;
			return this->allocate(n, hint);
		}
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
			delete [] page_pointer;
			delete next;
		}
		
	};

	size_t entity_size = sizeof(T);
	Page* head;
	Page* current;
 };


template<template<typename> class AllocatorType>
void allocator_benchmark(std::vector<char*>& words)
{
	std::map<char*, int, CStyleStringComparator,AllocatorType<std::pair<char*,int>>> map;
	auto start =std::chrono::high_resolution_clock::now();
	for (auto word : words)
	{
		auto iter = map.find(word);
		if (iter == map.end())
		{
			map.insert(std::pair<char*, int>(word, 1));
		}
		else
			iter->second++;
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Working time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " ms." << std::endl;
	
	/*for (const auto iter : map)
	{
		std::cout << iter.first << ": " << iter.second << std::endl;
	}*/
}


char* ReadFile(const char* filename,unsigned int& buffer_size)
{
	std::ifstream fileStream;
	fileStream.open(filename, std::ios_base::in);

	fileStream.seekg(0, std::ios_base::end);
	const int fileSize = fileStream.tellg();
	fileStream.seekg(0, std::ios_base::beg);
	buffer_size = fileSize;
	
	char* fileBuffer = new char[fileSize + 1];
	fileStream.read(fileBuffer, fileSize);
	fileBuffer[fileSize] = '\0';

	fileStream.close();
	return fileBuffer;
}

void split_book(std::vector<char*>& store,char* beggin, char* end)
{
	char* word_start = beggin;
	size_t word_size = 0;
	while (word_start <= end)
	{
		if(word_start[word_size] == ' ' || word_start[word_size] == '\n' || 
			word_start[word_size] == ',' || word_start[word_size] == ':' || 
			word_start[word_size] == '\0' || word_start[word_size] == '.'||
			word_start[word_size] == ';' || word_start[word_size] == '!')
		{
			if(word_size == 0)
			{
				word_start++;
			}
			else
			{
				store.push_back(word_start);
				word_start[word_size] = '\0';
				word_start = word_start + word_size;
				word_start++;
				word_size = 0;
			}
		}
		else
		{
			word_size++;
		}
	}
}


int main(int argc, char* argv[])
{
	std::vector<char*> words;
	unsigned int size;
	char* buffer = ReadFile("words.txt",size);

	split_book(words, buffer, buffer + size);

	std::cout << "Dummy Page Allocator" << std::endl;
	allocator_benchmark<DummyPageAllocator>(words);



	std::cout << std::endl;
	std::cout << "std::allocator" << std::endl;
	allocator_benchmark<std::allocator>(words);

	delete [] buffer;
	return 0;
}
