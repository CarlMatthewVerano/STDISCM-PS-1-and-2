#include "GraphConfig.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

GraphConfig::GraphConfig(std::string fileToExtractConfig) : fileToExtractConfig(fileToExtractConfig) {}


//std::string GraphConfig::trim(std::string& str)
//{
//	str.erase(str.find_last_not_of(' ') + 1);
//	str.erase(0, str.find_first_not_of(' '));
//	return str;
//}
//
//void GraphConfig::graphContentExtractor(std::string graphContent) {
//	graphContent = trim(graphContent);
//
//	if (graphContent.substr(0, 1) == "*") {
//		// This is a node
//		std::string node = graphContent.substr(2);
//		if (adjList.find(node) == adjList.end()) {
//			adjList[node] = std::vector<std::string>();
//		}
//	}
//	else if (graphContent.substr(0, 1) == "-") {
//		// This is an edge
//		size_t spacePos = graphContent.find(' ', 2);
//		std::string src = graphContent.substr(2, spacePos - 2);
//		std::string dest = graphContent.substr(spacePos + 1);
//		addEdge(src, dest);
//	}
//}

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
		adjList.emplace(node, std::vector<std::string>());
	}
	else if (trimmedContent[0] == '-') {
		// This is an edge
		size_t spacePos = trimmedContent.find(' ', 2);
		std::string src = trimmedContent.substr(2, spacePos - 2);
		std::string dest = trimmedContent.substr(spacePos + 1);
		addEdge(src, dest);
	}
}

void GraphConfig::addEdge(const std::string& src, const std::string& dest) {
	adjList[src].push_back(dest);
}


void GraphConfig::graphFileReader() {
	std::string graphContent;

	std::ifstream MyReadFile(fileToExtractConfig);

	while (getline(MyReadFile, graphContent)) {
		graphContentExtractor(graphContent);
	}

	MyReadFile.close();

	std::cout << "Graph File: graphFile has been loaded" << std::endl;
}