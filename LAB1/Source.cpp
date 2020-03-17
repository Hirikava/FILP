#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>

#include "dummy_page_allocator.hpp"


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

template<template<typename> class AllocatorType,typename ValueType, typename Comparator>
void allocator_benchmark(std::vector<ValueType>& words)
{
	std::map<ValueType, int, Comparator,AllocatorType<std::pair<ValueType,int>>> map;
	auto start =std::chrono::high_resolution_clock::now();
	for (auto word : words)
	{
		auto iter = map.find(word);
		if (iter == map.end())
		{
			map.insert(std::pair<ValueType, int>(word, 1));
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

template<typename ValueType>
void split_book(std::vector<ValueType>& store,char* beggin, char* end)
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
				word_start[word_size] = '\0';
				store.push_back(ValueType(word_start));
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


class StringCompare
{
public:
	bool operator()(const std::string s1,const std::string s2) const
	{
		return s1 < s2;
	}
};

int main(int argc, char* argv[])
{
	std::vector<char*> words;
	std::vector<std::string> words_str;
	
	unsigned int size;
	char* buffer = ReadFile("words.txt",size);

	split_book(words, buffer, buffer + size);
	split_book(words_str, buffer, buffer + size);

	std::cout << "Dummy Page Allocator and cahr*" << std::endl;
	allocator_benchmark<allocators::DummyPageAllocator,char*,CStyleStringComparator>(words);

	std::cout << std::endl;
	std::cout << "std::allocator and std::string" << std::endl;
	allocator_benchmark<std::allocator,std::string,StringCompare>(words_str);

	delete [] buffer;
	return 0;
}
