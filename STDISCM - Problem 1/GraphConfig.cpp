#include "GraphConfig.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

GraphConfig::GraphConfig(std::string fileToExtractConfig) : fileToExtractConfig(fileToExtractConfig) {}


std::string GraphConfig::trim(std::string& str)
{
	str.erase(str.find_last_not_of(' ') + 1);
	str.erase(0, str.find_first_not_of(' '));
	return str;
}

void GraphConfig::graphContentExtractor(std::string graphContent) {
	graphContent = trim(graphContent);

	if (graphContent.substr(0, 1) == "*") {
		// This is a node
		std::string node = graphContent.substr(2);
		if (adjList.find(node) == adjList.end()) {
			adjList[node] = std::vector<std::string>();
		}
	}
	else if (graphContent.substr(0, 1) == "-") {
		// This is an edge
		size_t spacePos = graphContent.find(' ', 2);
		std::string src = graphContent.substr(2, spacePos - 2);
		std::string dest = graphContent.substr(spacePos + 1);
		addEdge(src, dest);
	}
}

void GraphConfig::addEdge(std::string src, std::string dest) {
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