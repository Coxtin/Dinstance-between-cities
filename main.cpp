#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <list>
using namespace std;


struct Edge {
    int u;
    int v;
    int cost;
};

struct AdjEdge {
    int node;
    int cost;
};

class Graph {
    int n;            
    bool oriented;    

    vector< vector<AdjEdge> > adj;
    vector<string> cities;

public:
    Graph(int nodes, bool orientedFlag = false) : n(nodes), oriented(orientedFlag) {
        adj.resize(n);
        cities.resize(n);
    }

    void setCityName(int idx, const string &name) {
        if(idx >= 0 && idx < n)
            cities[idx] = name;
    }


    int getIndex(const string &name) {
        for (int i = 0; i < n; i++) {
            if (cities[i] == name)
                return i;
        }
        return -1; 
    }

    void addEdge(const string &cityU, const string &cityV, int cost) {
        int u = getIndex(cityU);
        int v = getIndex(cityV);
        if (u == -1 || v == -1) {
            cout << "Eroare: unul dintre orase nu a fost gasit: " << cityU << " sau " << cityV << "\n";
            return;
        }
        AdjEdge e;
        e.node = v;
        e.cost = cost;
        adj[u].push_back(e);
        if (!oriented) {
            e.node = u;
            adj[v].push_back(e);
        }
    }

    void findConnectedComponents() {
        vector<bool> visited(n, false);
        int componentCount = 0;
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                componentCount++;
                cout << "Componenta " << componentCount << ": ";
                queue<int> q;
                q.push(i);
                visited[i] = true;
                while (!q.empty()) {
                    int cur = q.front();
                    q.pop();
                    cout << cities[cur] << " ";
                    for (int j = 0; j < (int)adj[cur].size(); j++) {
                        int next = adj[cur][j].node;
                        if (!visited[next]) {
                            visited[next] = true;
                            q.push(next);
                        }
                    }
                }
                cout << "\n";
            }
        }
        cout << "Numar total de componente conexe: " << componentCount << "\n";
    }

    vector<int> dijkstra(const string &startCity) {
        int start = getIndex(startCity);
        vector<int> dist(n, 1000000000); 
        vector<bool> used(n, false);
        if (start == -1) {
            cout << "Orasul de start nu exista!\n";
            return dist;
        }
        dist[start] = 0;
        for (int i = 0; i < n; i++) {
            int u = -1;
            for (int j = 0; j < n; j++) {
                if (!used[j] && (u == -1 || dist[j] < dist[u]))
                    u = j;
            }
            if (u == -1)
                break;
            used[u] = true;
            for (int j = 0; j < (int)adj[u].size(); j++) {
                int v = adj[u][j].node;
                int cost = adj[u][j].cost;
                if (dist[u] + cost < dist[v])
                    dist[v] = dist[u] + cost;
            }
        }
        return dist;
    }

    void printDijkstra(const string &startCity) {
        int start = getIndex(startCity);
        if (start == -1) {
            cout << "Orasul de start nu exista!\n";
            return;
        }
        vector<int> dist = dijkstra(startCity);
        cout << "==== Cel mai scurt drum (Dijkstra) de la " << startCity << " ====\n";
        for (int i = 0; i < n; i++) {
            cout << "Distanta de la " << startCity << " la " << cities[i] << ": ";
            if (dist[i] >= 1000000000)
                cout << "N/A";
            else
                cout << dist[i];
            cout << "\n";
        }
    }

    int kruskalMST() {
        vector<Edge> edges;
        for (int u = 0; u < n; u++) {
            for (int j = 0; j < (int)adj[u].size(); j++) {
                int v = adj[u][j].node;
                int cost = adj[u][j].cost;
                if (u < v) {
                    Edge ed;
                    ed.u = u;
                    ed.v = v;
                    ed.cost = cost;
                    edges.push_back(ed);
                }
            }
        }

        for (int i = 0; i < (int)edges.size() - 1; i++) {
            int minIndex = i;
            for (int j = i + 1; j < (int)edges.size(); j++) {
                if (edges[j].cost < edges[minIndex].cost)
                    minIndex = j;
            }
            if (minIndex != i) {
                Edge temp = edges[i];
                edges[i] = edges[minIndex];
                edges[minIndex] = temp;
            }
        }

        vector<int> parent(n), rank(n, 0);
        for (int i = 0; i < n; i++) {
            parent[i] = i;
        }
        auto findSet = [&](int a) -> int {
            while (a != parent[a])
                a = parent[a];
            return a;
        };

        auto unionSet = [&](int a, int b) -> bool {
            a = findSet(a);
            b = findSet(b);
            if (a == b)
                return false;
            if (rank[a] < rank[b])
                parent[a] = b;
            else {
                parent[b] = a;
                if (rank[a] == rank[b])
                    rank[a]++;
            }
            return true;
        };

        int mstCost = 0;
        int edgesUsed = 0;
        for (int i = 0; i < (int)edges.size(); i++) {
            if (unionSet(edges[i].u, edges[i].v)) {
                mstCost += edges[i].cost;
                edgesUsed++;
                if (edgesUsed == n - 1)
                    break;
            }
        }
        return mstCost;
    }

    int edmondKarp(const string &sourceCity, const string &sinkCity) {
        int source = getIndex(sourceCity);
        int sink = getIndex(sinkCity);
        if (source == -1 || sink == -1) {
            cout << "Orasul sursa sau destinatie nu exista!\n";
            return 0;
        }

        vector< vector<int> > capacity(n, vector<int>(n, 0));
        for (int u = 0; u < n; u++) {
            for (int j = 0; j < (int)adj[u].size(); j++) {
                int v = adj[u][j].node;
                capacity[u][v] += adj[u][j].cost;
                if (!oriented)
                    capacity[v][u] += adj[u][j].cost;
            }
        }

        int maxFlow = 0;
        while (true) {
            vector<int> parent(n, -1);
            queue<int> q;
            q.push(source);
            parent[source] = source;
            while (!q.empty() && parent[sink] == -1) {
                int u = q.front();
                q.pop();
                for (int v = 0; v < n; v++) {
                    if (capacity[u][v] > 0 && parent[v] == -1) {
                        parent[v] = u;
                        q.push(v);
                    }
                }
            }
            if (parent[sink] == -1)
                break;

            int flow = 1000000000;
            int cur = sink;
            while (cur != source) {
                int prev = parent[cur];
                if (capacity[prev][cur] < flow)
                    flow = capacity[prev][cur];
                cur = prev;
            }

            cur = sink;
            while (cur != source) {
                int prev = parent[cur];
                capacity[prev][cur] -= flow;
                capacity[cur][prev] += flow;
                cur = prev;
            }
            maxFlow += flow;
        }
        return maxFlow;
    }

    void printAdjacencyList() {
        cout << "==== Lista de adiacenta ====\n";
        for (int i = 0; i < n; i++) {
            cout << cities[i] << ": ";
            for (int j = 0; j < (int)adj[i].size(); j++) {
                cout << "(" << cities[adj[i][j].node] << ", cost=" << adj[i][j].cost << ") ";
            }
            cout << "\n";
        }
    }

    void printAdjacencyMatrix() {
        cout << "==== Matricea de adiacenta ====\n";
        vector< vector<int> > matrix(n, vector<int>(n, 0));
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < (int)adj[i].size(); j++) {
                int v = adj[i][j].node;
                int cost = adj[i][j].cost;
                matrix[i][v] = cost;
            }
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                cout << matrix[i][j] << "\t";
            }
            cout << "\n";
        }
    }

    void printEdgeList() {
        cout << "==== Lista muchiilor ====\n";
        vector<Edge> edges;
        for (int u = 0; u < n; u++) {
            for (int j = 0; j < (int)adj[u].size(); j++) {
                int v = adj[u][j].node;
                int cost = adj[u][j].cost;
                if (!oriented && u > v)
                    continue;
                Edge ed;
                ed.u = u;
                ed.v = v;
                ed.cost = cost;
                edges.push_back(ed);
            }
        }
        for (int i = 0; i < (int)edges.size(); i++) {
            cout << cities[edges[i].u] << " - " << cities[edges[i].v] 
                 << " (cost = " << edges[i].cost << ")\n";
        }
    }

    void printIncidenceMatrix() {
        cout << "==== Matricea de incidenta ====\n";
        vector<Edge> edges;
        for (int u = 0; u < n; u++) {
            for (int j = 0; j < (int)adj[u].size(); j++) {
                int v = adj[u][j].node;
                int cost = adj[u][j].cost;
                if (!oriented && u > v)
                    continue;
                Edge ed;
                ed.u = u;
                ed.v = v;
                ed.cost = cost;
                edges.push_back(ed);
            }
        }
        int m = edges.size();
        vector< vector<int> > incMatrix(n, vector<int>(m, 0));
        for (int j = 0; j < m; j++) {
            incMatrix[edges[j].u][j] = 1;
            incMatrix[edges[j].v][j] = 1;
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                cout << incMatrix[i][j] << "\t";
            }
            cout << "\n";
        }
    }

    void DFSUtil(int node, vector<bool> &visited) {
        cout << cities[node] << " ";
        visited[node] = true;
        for (int j = 0; j < (int)adj[node].size(); j++) {
            int next = adj[node][j].node;
            if (!visited[next])
                DFSUtil(next, visited);
        }
    }

    void DFSPrint(const string &startCity) {
        int start = getIndex(startCity);
        if (start == -1) {
            cout << "Orasul de start nu exista!\n";
            return;
        }
        vector<bool> visited(n, false);
        cout << "DFS starting from " << startCity << ": ";
        DFSUtil(start, visited);
        cout << "\n";
    }

    void BFSPrint(const string &startCity) {
        int start = getIndex(startCity);
        if (start == -1) {
            cout << "Orasul de start nu exista!\n";
            return;
        }
        vector<bool> visited(n, false);
        queue<int> q;
        q.push(start);
        visited[start] = true;
        cout << "BFS starting from " << startCity << ": ";
        while (!q.empty()) {
            int node = q.front();
            q.pop();
            cout << cities[node] << " ";
            for (int j = 0; j < (int)adj[node].size(); j++) {
                int next = adj[node][j].node;
                if (!visited[next]) {
                    visited[next] = true;
                    q.push(next);
                }
            }
        }
        cout << "\n";
    }
};

int main() {
    ifstream fin("graf.txt");
    if (!fin) {
        cout << "Nu s-a putut deschide fisierul graf.txt\n";
        return 1;
    }

    int N, M;
    fin >> N >> M;

    Graph g(N, false);

    for (int i = 0; i < N; i++) {
        string city;
        fin >> city;
        g.setCityName(i, city);
    }

    for (int i = 0; i < M; i++) {
        string c1, c2;
        int cost;
        fin >> c1 >> c2 >> cost;
        g.addEdge(c1, c2, cost);
    }
    fin.close();

    cout << "==== Componente conexe ====\n";
    g.findConnectedComponents();
    cout << "\n";

    g.printDijkstra("Brasov");
    cout << "\n";

    cout << "==== Arbore partial de cost minim (Kruskal) ====\n";
    int mstCost = g.kruskalMST();
    cout << "Costul total al MST: " << mstCost << "\n\n";

    cout << "==== Flux maxim (Edmond-Karp) ====\n";
    int maxFlow = g.edmondKarp("Brasov", "Iasi");
    cout << "Fluxul maxim de la A la F este: " << maxFlow << "\n\n";

    g.printAdjacencyList();
    cout << "\n";
    g.printAdjacencyMatrix();
    cout << "\n";
    g.printEdgeList();
    cout << "\n";
    g.printIncidenceMatrix();
    cout << "\n";
    
    g.DFSPrint("Brasov");
    g.BFSPrint("Brasov");
    
    return 0;
}
