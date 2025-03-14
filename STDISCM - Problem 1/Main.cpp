#include "GraphConfig.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>


class ThreadPool {
public:
	ThreadPool(size_t numThreads) : stop(false) {
		for (size_t i = 0; i < numThreads; i++) {
			workers.emplace_back([this] {
				for (;;) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(queue_mutex);
						condition.wait(lock, [this] { return stop || !tasks.empty(); });
						if (stop && tasks.empty())
							return;
						task = std::move(tasks.front());
						tasks.pop();
					}
					task();
				}
				});
		}
	}
	template<class F>
	void enqueue(F f) {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.push(std::function<void()>(f));
		}
		condition.notify_one();
	}
	~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers)
			worker.join();
	}
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
};

bool isPrime(int n) {
	if (n < 2) return false;
	for (int i = 2; i * i <= n; i++) {
		if (n % i == 0)
			return false;
	}
	return true;
}


void dfsShortestPrimePath(const std::string& current, const std::string& dest,
	const std::map<std::string, std::vector<std::pair<std::string, int>>>& adjList,
	std::vector<std::string>& currentPath, int currentCost,
	int& bestCost, std::vector<std::string>& bestPath, std::mutex& bestMutex, const std::string& option, const bool& isParallel)
{
	if (current == dest) {
		if (isParallel) {
			std::lock_guard<std::mutex> lock(bestMutex);
		}

		if (option == "Path") {
			bestCost = currentCost;
			bestPath = currentPath;
		}
		else if (option == "Shortest") {
			if (currentCost < bestCost) {
				bestCost = currentCost;
				bestPath = currentPath;
			}
		}
		else if (option == "Prime") {
			if (isPrime(currentCost)) {
				bestCost = currentCost;
				bestPath = currentPath;
			}
		}
		else if (option == "ShortestPrime") {
			if (isPrime(currentCost)) {
				if (currentCost < bestCost) {
					bestCost = currentCost;
					bestPath = currentPath;
				}
			}
		}
		return;
	}

	if (adjList.find(current) == adjList.end()) {
		return;
	}

	for (const auto& edge : adjList.at(current)) {
		const std::string& next = edge.first;
		int weight = edge.second;
		if (std::find(currentPath.begin(), currentPath.end(), next) != currentPath.end())
			continue;
		int newCost = currentCost + weight;
		currentPath.push_back(next);
		dfsShortestPrimePath(next, dest, adjList, currentPath, newCost, bestCost, bestPath, bestMutex, option, isParallel);
		currentPath.pop_back();
	}
}


std::string queryExtract(const std::string& query, const std::string& option) {
	if (option == "Path") {
		return query.substr(5);
	}
	else if (option == "Shortest") {
		return query.substr(14);
	}
	else if (option == "Prime") {
		return query.substr(11);
	}
	else if (option == "ShortestPrime") {
		return query.substr(20);
	}
}

void printOption(const std::string& option) {
	if (option == "Path") {
		std::cout << "path: ";
	}
	else if (option == "Shortest") {
		std::cout << "shortest path: ";
	}
	else if (option == "Prime") {
		std::cout << "prime path: ";
	}
	else if (option == "ShortestPrime") {
		std::cout << "shortest prime path: ";
	}
}

void checkShortestPrimePathInGraph(const std::map<std::string, std::vector<std::pair<std::string, int>>>& adjList, const std::string& query, const std::string& option) {
	std::string pathQuery = queryExtract(query, option);
	size_t spacePos = pathQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID SHORTEST PATH QUERY" << std::endl;
		return;
	}
	std::string src = pathQuery.substr(0, spacePos);
	std::string dest = pathQuery.substr(spacePos + 1);

	if (adjList.find(src) == adjList.end()) {
		std::cout << "No path from " << src << " to " << dest << std::endl;
		return;
	}

	int bestCost = INT_MAX;
	std::vector<std::string> bestPath;
	std::vector<std::string> currentPath = { src };
	std::mutex bestMutex;

	dfsShortestPrimePath(src, dest, adjList, currentPath, 0, bestCost, bestPath, bestMutex, option, false);

	if (!bestPath.empty()) {
		printOption(option);
		for (size_t i = 0; i < bestPath.size(); i++) {
			std::cout << bestPath[i] << (i != bestPath.size() - 1 ? " -> " : "");
		}
		std::cout << " with weight/length= " << bestCost << std::endl;
	}
	else {
		std::cout << "No path from " << src << " to " << dest << std::endl;
	}
}

void checkShortestPrimePathInGraphParallel(const std::map<std::string, std::vector<std::pair<std::string, int>>>& adjList, const std::string& query, const std::string& option) {
	std::string pathQuery = queryExtract(query, option);
	size_t spacePos = pathQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID SHORTEST PATH QUERY" << std::endl;
		return;
	}
	std::string src = pathQuery.substr(0, spacePos);
	std::string dest = pathQuery.substr(spacePos + 1);

	if (adjList.find(src) == adjList.end()) {
		std::cout << "No path from " << src << " to " << dest << std::endl;
		return;
	}

	int bestCost = INT_MAX;
	std::vector<std::string> bestPath;
	std::mutex bestMutex;
	std::vector<std::string> currentPath = { src };

	const size_t numThreads = 6;
	{
		ThreadPool pool(numThreads);
		for (const auto& edge : adjList.at(src)) {
			std::string neighbor = edge.first;
			int weight = edge.second;
			std::vector<std::string> threadPath = currentPath;
			threadPath.push_back(neighbor);
			pool.enqueue([=, &adjList, &bestCost, &bestPath, &bestMutex]() mutable {
				dfsShortestPrimePath(neighbor, dest, adjList, threadPath, weight, bestCost, bestPath, bestMutex, option, true);
				});
		}
	}

	if (!bestPath.empty()) {
		printOption(option);
		for (size_t i = 0; i < bestPath.size(); i++) {
			std::cout << bestPath[i] << (i != bestPath.size() - 1 ? " -> " : "");
		}
		std::cout << " with weight/length= " << bestCost << std::endl;
	}
	else {
		std::cout << "No path from " << src << " to " << dest << std::endl;
	}
}

static void edgesFormatter(const std::string& edgeToFormat, bool isLast) {
	size_t firstSpacePos = edgeToFormat.find(' ');
	size_t secondSpacePos = edgeToFormat.find(' ', firstSpacePos + 1);
	std::string node1 = edgeToFormat.substr(0, firstSpacePos);
	std::string node2 = edgeToFormat.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
	std::string weight = edgeToFormat.substr(secondSpacePos + 1);

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
				checkShortestPrimePathInGraphParallel(adjList, query, "Path");
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				checkShortestPrimePathInGraph(adjList, query, "Path");
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
		}
		else if (query.find("prime-path ") == 0) {
			if (parallel == true) {
				auto start = std::chrono::high_resolution_clock::now();
				checkShortestPrimePathInGraphParallel(adjList, query, "Prime");
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				checkShortestPrimePathInGraph(adjList, query, "Prime");
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
		}
		else if (query.find("shortest-path ") == 0) {
			if (parallel == true) {
				auto start = std::chrono::high_resolution_clock::now();
				checkShortestPrimePathInGraphParallel(adjList, query, "Shortest");
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				checkShortestPrimePathInGraph(adjList, query, "Shortest");
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
		}
		else if (query.find("shortest-prime-path ") == 0) {
			if (parallel == true) {
				auto start = std::chrono::high_resolution_clock::now();
				checkShortestPrimePathInGraphParallel(adjList, query, "ShortestPrime");
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				std::cout << "Execution time: " << elapsed.count() << " ms" << std::endl;
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				checkShortestPrimePathInGraph(adjList, query, "ShortestPrime");
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