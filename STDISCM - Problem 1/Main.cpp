#include "GraphConfig.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

//void printGraph(std::map<std::string, std::vector<std::string>> adjList) {
//	for (const auto& pair : adjList) {
//		std::cout << pair.first << ": ";
//		for (const auto& vertex : pair.second) {
//			std::cout << vertex << " -> ";
//		}
//		std::cout << "NULL" << std::endl;
//	}
//
//}

static void edgesFormatter(const std::string& edgeToFormat, bool isLast) {
	size_t spacePos = edgeToFormat.find(' ');
	std::string node1 = edgeToFormat.substr(0, spacePos);
	std::string node2 = edgeToFormat.substr(spacePos + 1);

	std::cout << "(" << node1 << "," << node2 << ")";
	if (!isLast) {
		std::cout << ", ";
	}
	else {
		std::cout << std::endl;
	}
}

void printNodes(std::map<std::string, std::vector<std::string>> adjList) {
	for (const auto& pair : adjList) {
		std::cout << pair.first << " ";
	}
	std::cout << std::endl;
}

void printEdges(std::map<std::string, std::vector<std::string>> adjList) {
	for (const auto& pair : adjList) {
		for (const auto& vertex : pair.second) {
			edgesFormatter(pair.first + " " + vertex, pair.first == adjList.rbegin()->first && vertex == pair.second.back());
		}
	}
}

void checkNodeExistInGraph(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string nodeQuery = query.substr(5);

	bool nodeExists = false;
	for (auto i = adjList.begin(); i != adjList.end(); i++) {
		if (i->first == nodeQuery) {
			nodeExists = true;
			break;
		}
	}
	if (nodeExists) {
		std::cout << "Node " << nodeQuery << " is in the graph" << std::endl;
	}
	else {
		std::cout << "Node " << nodeQuery << " is not in the graph" << std::endl;
	}
}

void nodeSearcher(const std::map<std::string, std::vector<std::string>>& adjList, const std::string& nodeQuery, std::atomic<bool>& nodeExists, size_t start, size_t end, std::string runner) {
	auto it = adjList.begin();
	std::advance(it, start);
	for (size_t i = start; i < end && !nodeExists; i++, it++) {
		std::cout << runner << " running from " << start + 1 << " to " << end << std::endl;
		if (it->first == nodeQuery) {
			nodeExists = true;
			return;
		}
	}
}

void checkNodeExistInGraphParallel(const std::map<std::string, std::vector<std::string>>& adjList, const std::string& query) {
	std::string nodeQuery = query.substr(5);

	std::vector<std::thread> workerThreads;
	int numThreads = 6;

	size_t totalSize = adjList.size();
	size_t baseSize = totalSize / numThreads;
	size_t remainder = totalSize % numThreads;

	std::atomic<bool> nodeExists(false);

	size_t start = 0;
	for (int i = 0; i < numThreads; i++) {
		size_t end = start + baseSize + (i < remainder ? 1 : 0);
		workerThreads.emplace_back([&, start, end, i]() {
			std::cout << "Thread " << i << " started" << std::endl;
			nodeSearcher(adjList, nodeQuery, nodeExists, start, end, "T- " + std::to_string(i));
			});
		start = end;
	}

	for (auto& t : workerThreads) {
		t.join();
	}

	if (nodeExists) {
		std::cout << "Node " << nodeQuery << " is in the graph" << std::endl;
	}
	else {
		std::cout << "Node " << nodeQuery << " is not in the graph" << std::endl;
	}
}

void checkEdgeExistInGraph(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string edgeQuery = query.substr(5);
	size_t spacePos = edgeQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID EDGE QUERY" << std::endl;
		return;
	}

	std::string src = edgeQuery.substr(0, spacePos);
	std::string dest = edgeQuery.substr(spacePos + 1);

	bool found = false;

	if (adjList.find(src) != adjList.end()) {
		if (std::find(adjList[src].begin(), adjList[src].end(), dest) != adjList[src].end()) {
			std::cout << "Edge (" << src << "," << dest << ") is in the graph" << std::endl;
			found = true;
		}
	}

	if (!found) {
		std::cout << "Edge (" << src << "," << dest << ") is not in the graph" << std::endl;
	}
}

void edgeSearcher(const std::map<std::string, std::vector<std::string>>& adjList, const std::string& src, const std::string& dest, std::atomic<bool>& edgeExists, size_t start, size_t end) {
	auto it = adjList.begin();
	std::advance(it, start);
	for (size_t i = start; i < end && !edgeExists; ++i, ++it) {
		if (it->first == src) {
			if (std::find(it->second.begin(), it->second.end(), dest) != it->second.end()) {
				edgeExists = true;
				return;
			}
		}
	}
}

void checkEdgeExistInGraphParallel(const std::map<std::string, std::vector<std::string>>& adjList, const std::string& query) {
	std::vector<std::thread> workerThreads;
	int numThreads = 6;

	size_t totalSize = adjList.size();
	size_t baseSize = totalSize / numThreads;
	size_t remainder = totalSize % numThreads;

	std::atomic<bool> edgeExists(false);

	std::string edgeQuery = query.substr(5);
	size_t spacePos = edgeQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID EDGE QUERY" << std::endl;
		return;
	}

	std::string src = edgeQuery.substr(0, spacePos);
	std::string dest = edgeQuery.substr(spacePos + 1);

	size_t start = 0;
	for (int i = 0; i < numThreads; ++i) {
		size_t end = start + baseSize + (i < remainder ? 1 : 0);
		workerThreads.emplace_back(edgeSearcher, std::cref(adjList), std::cref(src), std::cref(dest), std::ref(edgeExists), start, end);
		std::cout << "Thread " << i << " running from " << start << " to " << end - 1 << std::endl;
		start = end;
	}

	for (auto& t : workerThreads) {
		t.join();
	}

	if (edgeExists) {
		std::cout << "Edge (" << src << "," << dest << ") is in the graph" << std::endl;
	}
	else {
		std::cout << "Edge (" << src << "," << dest << ") is not in the graph" << std::endl;
	}
}

static bool dfsUtil(const std::string& src, const std::string& dest, std::unordered_set<std::string>& visited, std::vector<std::string>& path, std::map<std::string, std::vector<std::string>> adjList) {
	visited.insert(src);
	path.push_back(src);

	if (src == dest) {
		return true;
	}

	for (const std::string& neighbor : adjList[src]) {
		if (visited.find(neighbor) == visited.end()) {
			if (dfsUtil(neighbor, dest, visited, path, adjList)) {
				return true;
			}
		}
	}

	path.pop_back();
	return false;
}

static bool findPathDFS(const std::string& src, const std::string& dest, std::vector<std::string>& path, std::map<std::string, std::vector<std::string>> adjList) {
	std::unordered_set<std::string> visited;
	return dfsUtil(src, dest, visited, path, adjList);
}

static void checkPathExistInGraph(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string pathQuery = query.substr(5);
	size_t spacePos = pathQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID PATH QUERY" << std::endl;
		return;
	}

	std::string src = pathQuery.substr(0, spacePos);
	std::string dest = pathQuery.substr(spacePos + 1);

	std::vector<std::string> path;

	if (findPathDFS(src, dest, path, adjList)) {
		std::cout << "Path from " << src << " to " << dest << ": ";
		for (const std::string& node : path) {
			if (node != path.back()) {
				std::cout << node << " -> ";
			}
			else {
				std::cout << node;
			}
		}
		std::cout << std::endl;
	}
	else {
		std::cout << "No path found from " << src << " to " << dest << std::endl;
	}
}

static void checkPathExistInGraphParallel(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string pathQuery = query.substr(5);
	size_t spacePos = pathQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID PATH QUERY" << std::endl;
		return;
	}

	std::string src = pathQuery.substr(0, spacePos);
	std::string dest = pathQuery.substr(spacePos + 1);

	std::vector<std::thread> workerThreads;
	int numThreads = 6;

	size_t totalSize = adjList.size();
	size_t baseSize = totalSize / numThreads;
	size_t remainder = totalSize % numThreads;

	std::atomic<bool> pathFound(false);
	std::vector<std::string> path;
	size_t start = 0;
	for (int i = 0; i < numThreads; ++i) {
		size_t end = start + baseSize + (i < remainder ? 1 : 0);
		workerThreads.emplace_back([&, start, end, i]() {
			std::cout << "Thread " << i << " running from " << start << " to " << end - 1 << std::endl;
			std::unordered_set<std::string> visited;
			std::vector<std::string> localPath;
			auto it = adjList.begin();
			std::advance(it, start);
			for (size_t j = start; j < end && !pathFound; ++j, ++it) {
				if (dfsUtil(it->first, dest, visited, localPath, adjList)) {
					pathFound = true;
					localPath.insert(localPath.begin(), src);
					path = localPath;
					return;
				}
			}
			});
		start = end;
	}

	for (auto& t : workerThreads) {
		t.join();
	}

	if (pathFound) {
		std::cout << "Path from " << src << " to " << dest << ": ";
		for (const std::string& node : path) {
			if (node != path.back()) {
				std::cout << node << " -> ";
			}
			else {
				std::cout << node;
			}

		}
		std::cout << std::endl;
	}
	else {
		std::cout << "No path found from " << src << " to " << dest << std::endl;
	}
}

static void queries(std::map<std::string, std::vector<std::string>> adjList) {
	bool parallel = false;
	std::string query;
	while (true)
	{
		std::cout << "Graph Query: ";
		getline(std::cin, query);

		std::cout << "Response: ";

		if (query == "exit") {
			break;
		}
		else if (query == "parallel") {
			parallel = !parallel;
		}
		else if (query == "nodes") {
			printNodes(adjList);
		}
		else if (query.find("node ") == 0) {
			std::cout << std::endl;
			if (parallel == true) {
				checkNodeExistInGraphParallel(adjList, query);
			}
			else {
				checkNodeExistInGraph(adjList, query);
			}
		}
		else if (query == "edges") {
			printEdges(adjList);
		}
		else if (query.find("edge ") == 0) {
			if (parallel == true) {
				checkEdgeExistInGraphParallel(adjList, query);
			}
			else {
				checkEdgeExistInGraph(adjList, query);
			}
		}
		else if (query.find("path ") == 0) {
			if (parallel == true) {
				checkPathExistInGraphParallel(adjList, query);
			}
			else {
				checkPathExistInGraph(adjList, query);
			}
		}
		else {
			std::cout << "INVALID QUERY" << std::endl;
		}

	}
}


int main()
{
	GraphConfig graphConfig("graphFile100.txt");
	graphConfig.graphFileReader();
	queries(graphConfig.adjList);
}