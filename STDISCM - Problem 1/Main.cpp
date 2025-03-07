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
	size_t firstSpacePos = edgeToFormat.find(' ');
	size_t secondSpacePos = edgeToFormat.find(' ', firstSpacePos + 1);
	std::string node1 = edgeToFormat.substr(0, firstSpacePos);
	std::string node2 = edgeToFormat.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
	std::string weight = edgeToFormat.substr(secondSpacePos + 1);

	//size_t spacePos = edgeToFormat.find(' ');
	//std::string node1 = edgeToFormat.substr(0, spacePos);
	//std::string node2 = edgeToFormat.substr(spacePos + 1);

	std::cout << "(" << node1 << "," << node2 << "," << weight << ")";
	if (!isLast) {
		std::cout << ", ";
	}
	else {
		std::cout << std::endl;
	}
}

void printNodes(std::map<std::string, std::vector<std::pair<std::string, int>>> adjList) {
	for (const auto& pair : adjList) {
		std::cout << pair.first << " ";
	}
	std::cout << std::endl;
}

void printEdges(std::map<std::string, std::vector<std::pair<std::string, int>>> adjList) {
	for (const auto& pair : adjList) {
		for (const auto& vertex : pair.second) {
			edgesFormatter(pair.first + " " + vertex.first + " " + std::to_string(vertex.second), pair.first == adjList.rbegin()->first && vertex.first == pair.second.back().first);
		}
	}
}

void checkNodeExistInGraph(std::map<std::string, std::vector<std::pair<std::string, int>>> adjList, const std::string& query) {
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

void nodeSearcher(const std::map<std::string, std::vector<std::pair<std::string, int>>>& adjList, const std::string& nodeQuery, std::atomic<bool>& nodeExists, size_t start, size_t end) {
	auto it = adjList.begin();
	std::advance(it, start);
	//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	for (size_t i = start; i < end && !nodeExists; ++i, ++it) {
		if (it->first == nodeQuery) {
			nodeExists = true;
			return;
		}
	}
}

void checkNodeExistInGraphParallel(const std::map<std::string, std::vector<std::pair<std::string, int>>>& adjList, const std::string& query) {
	std::string nodeQuery = query.substr(5);

	std::vector<std::thread> workerThreads;
	int numThreads = 6;

	size_t totalSize = adjList.size();
	size_t baseSize = totalSize / numThreads;
	size_t remainder = totalSize % numThreads;

	std::atomic<bool> nodeExists(false);

	size_t start = 0;
	for (int i = 0; i < numThreads; ++i) {
		size_t end = start + baseSize + (i < remainder ? 1 : 0);
		workerThreads.emplace_back(nodeSearcher, std::cref(adjList), nodeQuery, std::ref(nodeExists), start, end);
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

void checkEdgeExistInGraph(std::map<std::string, std::vector<std::pair<std::string, int>>> adjList, const std::string& query) {
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
		for (const auto& edge : adjList.at(src)) {
			if (edge.first == dest) {
				std::cout << "Edge (" << src << ", " << dest << ") is in the graph" << std::endl;
				found = true;
				break;
			}
		}
	}

	if (!found) {
		std::cout << "Edge (" << src << "," << dest << ") is not in the graph" << std::endl;
	}
}

void edgeSearcher(const std::map<std::string, std::vector<std::pair<std::string, int>>>& adjList, const std::string& src, const std::string& dest, std::atomic<bool>& edgeExists, size_t start, size_t end) {
	auto it = adjList.begin();
	std::advance(it, start);
	//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	for (size_t i = start; i < end && !edgeExists; ++i, ++it) {
		if (it->first == src) {
			for (const auto& edge : it->second) {
				if (edge.first == dest) {
					edgeExists = true;
					return;
				}
			}
		}
	}
}

void checkEdgeExistInGraphParallel(const std::map<std::string, std::vector<std::pair<std::string, int>>>& adjList, const std::string& query) {
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

static bool dfsUtil(const std::string& src, const std::string& dest, std::unordered_set<std::string>& visited, std::vector<std::string>& path, const std::map<std::string, std::vector<std::pair<std::string, int>>> adjList) {
	visited.insert(src);
	path.push_back(src);

	if (src == dest) {
		return true;
	}

	for (const auto& neighbor : adjList.at(src)) {
		if (visited.find(neighbor.first) == visited.end()) {
			if (dfsUtil(neighbor.first, dest, visited, path, adjList)) {
				return true;
			}
		}
	}

	path.pop_back();
	return false;
}

static bool findPathDFS(const std::string& src, const std::string& dest, std::vector<std::string>& path, std::map<std::string, std::vector<std::pair<std::string, int>>> adjList) {
	std::unordered_set<std::string> visited;
	return dfsUtil(src, dest, visited, path, adjList);
}

static void checkPathExistInGraph(std::map<std::string, std::vector<std::pair<std::string, int>>> adjList, const std::string& query) {
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

void checkPathExistInGraphParallel(const std::map<std::string, std::vector<std::pair<std::string, int>>> adjList, const std::string& query) {
	std::string pathQuery = query.substr(5);
	size_t spacePos = pathQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID PATH QUERY" << std::endl;
		return;
	}

	std::string src = pathQuery.substr(0, spacePos);
	std::string dest = pathQuery.substr(spacePos + 1);

	if (adjList.find(src) == adjList.end()) {
		std::cout << "No path found from " << src << " to " << dest << std::endl;
		return;
	}

	std::atomic<bool> pathFound(false);
	std::mutex pathMutex;
	std::vector<std::string> finalPath;
	std::vector<std::thread> workerThreads;

	for (const auto& neighborPair : adjList.at(src)) {
		const std::string& neighbor = neighborPair.first;
		workerThreads.emplace_back([&, neighbor = neighbor]() {
			std::unordered_set<std::string> visited{ src };
			std::vector<std::string> localPath;

			if (dfsUtil(neighbor, dest, visited, localPath, adjList)) {
				std::lock_guard<std::mutex> lock(pathMutex);
				if (!pathFound) {
					finalPath.push_back(src);
					finalPath.insert(finalPath.end(), localPath.begin(), localPath.end());
					pathFound = true;
				}
			}
			});
	}

	for (auto& thread : workerThreads) {
		thread.join();
	}

	if (pathFound) {
		for (size_t i = 0; i < finalPath.size(); ++i) {
			std::cout << finalPath[i];
			if (i < finalPath.size() - 1) std::cout << " -> ";
		}
		std::cout << std::endl;
	}
	else {
		std::cout << "No path found from " << src << " to " << dest << std::endl;
	}
}

static void queries(std::map<std::string, std::vector<std::pair<std::string, int>>> adjList) {
	bool parallel = false;
	std::string query;


	std::cout << " _____ ______  ___  ______ _   _        _____ _   _ _____________   __" << std::endl;
	std::cout << "|  __ \\| ___ \\/ _ \\ | ___ \\ | | |      |  _  | | | |  ___| ___ \\ \\ / /" << std::endl;
	std::cout << "| |  \\/| |_/ / /_\\ \\| |_/ / |_| |      | | | | | | | |__ | |_/ /\\ V / " << std::endl;
	std::cout << "| | __ |    /|  _  ||  __/|  _  |      | | | | | | |  __||    /  \\ /  " << std::endl;
	std::cout << "| |_\\ \\| |\\ \\| | | || |   | | | |      \\ \\/' / |_| | |___| |\\ \\  | |  " << std::endl;
	std::cout << " \\____/\\_| \\_\\_| |_/\\_|   \\_| |_/       \\_/\\_\\\\___/\\____/\\_| \\_| \\_/ " << std::endl;
	std::cout << std::endl;

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
			std::cout << "Parallel mode is now " << (parallel ? "ON" : "OFF") << std::endl;
		}
		else if (query == "nodes") {
			printNodes(adjList);
		}
		else if (query.find("node ") == 0) {
			std::cout << std::endl;
			if (parallel == true) {
				auto start = std::chrono::high_resolution_clock::now();
				checkNodeExistInGraphParallel(adjList, query);
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				checkNodeExistInGraph(adjList, query);
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
		}
		else if (query == "edges") {
			printEdges(adjList);
		}
		else if (query.find("edge ") == 0) {
			if (parallel == true) {
				auto start = std::chrono::high_resolution_clock::now();
				checkEdgeExistInGraphParallel(adjList, query);
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				checkEdgeExistInGraph(adjList, query);
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
		}
		else if (query.find("path ") == 0) {
			if (parallel == true) {
				auto start = std::chrono::high_resolution_clock::now();
				checkPathExistInGraphParallel(adjList, query);
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				checkPathExistInGraph(adjList, query);
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
		}
		else {
			std::cout << "INVALID QUERY" << std::endl;
		}

	}
}


int main()
{
	GraphConfig graphConfig("graphFile.txt");
	graphConfig.graphFileReader();
	queries(graphConfig.adjList);
}