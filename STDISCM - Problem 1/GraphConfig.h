//#pragma once

#ifndef GRAPHCONFIG_H
#define GRAPHCONFIG_H

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class GraphConfig {
public:
	std::string trim(std::string& str);
	std::map<std::string, std::vector<std::pair<std::string, int>>> adjList;
	std::string fileToExtractConfig;
	GraphConfig(std::string fileToExtractConfig);
	void graphContentExtractor(const std::string& graphContent);

	void graphFileReader();
	void addEdge(const std::string& src, const std::string& dest, int weight);
};

#endif // GRAPHCONFIG_H