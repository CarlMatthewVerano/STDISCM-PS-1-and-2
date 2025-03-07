#include "GraphConfig.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

GraphConfig::GraphConfig(std::string fileToExtractConfig) : fileToExtractConfig(fileToExtractConfig) {}

std::string GraphConfig::trim(std::string& str) {
	auto start = str.find_first_not_of(' ');
	auto end = str.find_last_not_of(' ');
	return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

void GraphConfig::graphContentExtractor(const std::string& graphContent) {
	std::string trimmedContent = trim(const_cast<std::string&>(graphContent));

	if (trimmedContent.empty()) return;

	if (trimmedContent[0] == '*') {
		// This is a node
		std::string node = trimmedContent.substr(2);
		adjList.emplace(node, std::vector<std::pair<std::string, int>>());
	}
	else if (trimmedContent[0] == '-') {
		// This is an edge
		size_t firstSpacePos = trimmedContent.find(' ', 2);
		size_t secondSpacePos = trimmedContent.find(' ', firstSpacePos + 1);
		std::string src = trimmedContent.substr(2, firstSpacePos - 2);
		std::string dest = trimmedContent.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
		int weight = std::stoi(trimmedContent.substr(secondSpacePos + 1));
		addEdge(src, dest, weight);
	}
}

void GraphConfig::addEdge(const std::string& src, const std::string& dest, int weight) {
	adjList[src].emplace_back(dest, weight);
}


void GraphConfig::graphFileReader() {
	std::string graphContent;

	std::ifstream MyReadFile(fileToExtractConfig);

	while (getline(MyReadFile, graphContent)) {
		graphContentExtractor(graphContent);
	}

	MyReadFile.close();

	std::cout << "Graph File: graphFile has been loaded" << std::endl;

	/*for (const auto& pair : adjList) {
		std::cout << "Source: " << pair.first << std::endl;
		for (const auto& dest : pair.second) {
			std::cout << "     Destination: " << dest.first << "  --  Weight: " << dest.second << std::endl;
		}
	}*/
}