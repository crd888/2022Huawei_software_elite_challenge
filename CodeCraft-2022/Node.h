#ifndef _NODE_H
#define _NODE_H
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
using namespace std;
// �����ļ�
class Config {
private:
    const char* qosConstraint = "qos_constraint";
    int constraint;
public:
    Config();
    Config(const char* file);
    void showMsg();
    int getConstraint();
};
// ��Ե�ڵ㼯��
class Edge {
private:
    int edgeSize;
public:
    std::vector<int> bandwidth;
    std::unordered_map<std::string, int> indexEdge;
    std::vector<int> leftBandwidth;
    std::vector<std::string> name;
    Edge();
    Edge(const char* file);
    void showMsg();
    int size();
    void reStart();
};
// �ͻ��ڵ���Ϣ
class ClientNode {
public:
    int index;
    std::string name;
    std::vector<int> demand;
    int leftDemand;
    // �ͻ��˵Ĺ��캯��
    ClientNode() {
        index = 0;
    }
    ClientNode(int _index, std::string _name) :
        index(_index),
        name(_name) {

    }
};
// �ͻ��ڵ㼯��
class Client {
private:
    int clientSize;
public:
    std::unordered_map<std::string, int> indexClient;      // ���ֶ�Ӧ������
    std::vector<std::string> timeList;  // ʱ������
    std::vector<ClientNode> clientNodeList;
    Client();
    Client(const char* file);
    void showMsg();
    int size();
    void reStart(int k);
};
//
class Solution {
public:
    Config config;
    Edge edge;
    Client client;
private:
    int N;                                      // N��Ե�ڵ�����
    int M;                                      // M�ͻ��ڵ�����
    int T;
    int timeLen;
    int constraint;                             // ��������ӳ�����
    std::ofstream fout;                         // ��������ļ�
public:
    // ��¼�ͻ��ڵ�i����Ե�ڵ�j���ӳ� �� M��N��
    std::vector<std::vector<int>> delay;
    // ��¼�ͻ��ڵ�i����Ե�ڵ�j�Ƿ����ʹ�ã�����Ϊ1������Ϊ0 �� M��N��
    std::vector<std::vector<int>> access;
    // ��¼��Ե�ڵ�i����ʹ�õĿͻ��ڵ���ţ������ܷ����������С�������� �� N��
    std::vector<std::vector<int>> edge2Client;
    // ��¼�ͻ��ڵ�j����ʹ�õı�Ե�ڵ���ţ����������������С�������� �� M��
    std::vector<std::vector<int>> client2Edge;
    // ��¼��Ե�ڵ����˳���ڹ��캯������ɼ��㣬�������ٵĿ�ʼ����
    std::vector<int> edgeAllocationSequence;
    // ��¼�ͻ��ڵ���Ҫ˳���ڹ��캯������ɼ��㣬�������ٵĿ�ʼ����
    std::vector<int> clientGetSequence;
    // ��Ե�ڵ�Ⱦɫ���
    vector<vector<int>> group;
    // 
    std::vector<std::vector<pair<int, int>>> historyAlloc;
    Solution();
    ~Solution();
    //Edge2ClientDelay(Edge &edge, Client &client, Config &config);
    void showBaseMatrixMsg();
    void showClientCanUse();
    void showEdgeCanUse();
    void showClientDemand();
    //void dfs(vector<int> &mark, int start, int clr);
    //void getColor();
    int getN() { return N; }
    int getM() { return M; }
    void startWork();
    void allocFunc();
    void reAllocFunc(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int index);
    void outputSolution(std::vector<std::vector<int>>& allot, int k);
    void allocBiggest5_1(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int biggerNum);
    void allocBiggest5_2(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int biggerNum);
    void allocBiggest5_3(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int biggerNum);
    void allocBiggest5_4(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int biggerNum);
    void allocBiggest5_5(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int biggerNum);
    void allocBiggest5_6(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int biggerNum);
    void allocBiggest_48w(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand, int biggerNum);
    void doAllocOpt_48w(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand);
    void doAllocOptByAveDemand(vector<vector<vector<int>>>& allot, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& clientLeftDemand);
};



struct DinicEdge {
    int from, to, cap, flow;
    DinicEdge(int u, int v, int c, int f) : from(u), to(v), cap(c), flow(f) {}
};
struct Dinic {
#define maxn 250 // count of Node
#define INF 0x3f3f3f3f
    int m;
    int s;//源点
    int t;//汇点
    vector<DinicEdge> edges;
    vector<int> G[maxn];//网络容量
    int d[maxn];//终点
    int cur[maxn];//起点
    bool vis[maxn];//深度


    void init(int n) {
        for (int i = 0; i < n; i++) G[i].clear();
        edges.clear();
    }

    void AddEdge(int from, int to, int cap) 
    {
        edges.push_back(DinicEdge(from, to, cap, 0));//正向边
        edges.push_back(DinicEdge(to, from, 0, 0));//反向边
        m = edges.size();
        G[from].push_back(m - 2);
        G[to].push_back(m - 1);
    }

    void AddDoubleEdge(int from, int to, int cap) {
        edges.push_back(DinicEdge(from, to, cap, 0));
        edges.push_back(DinicEdge(to, from, cap, 0));
        m = edges.size();
        G[from].push_back(m - 2);//from所能出去的边
        G[to].push_back(m - 1);//to所能进来的边
    }

    //建立分层图
    bool BFS() {
        memset(vis, 0, sizeof(vis));
        queue<int> Q;
        Q.push(s);
        d[s] = 0;
        vis[s] = 1;
        while (!Q.empty()) {
            int x = Q.front();
            Q.pop();
            for (int i = 0; i < G[x].size(); i++) {
                DinicEdge& e = edges[G[x][i]];
                if (!vis[e.to] && e.cap > e.flow) {
                    vis[e.to] = 1;
                    d[e.to] = d[x] + 1;
                    Q.push(e.to);
                }
            }
        }
        return vis[t];//是否访问到t，false表明已经流完了
    }

    //增广
    int DFS(int x, int a) 
    {
        //当前点已经到达终点
        if (x == t || a == 0) return a;
        int flow = 0;
        //遍历当前点可以出去的边
        for (int& i = cur[x]; i < G[x].size(); i++) 
        {
            //获取该条边
            DinicEdge& e = edges[G[x][i]];
            int f;
            if (d[x] + 1 == d[e.to] && (f = DFS(e.to, min(a, e.cap - e.flow))) > 0) {
                e.flow += f;
                edges[G[x][i] ^ 1].flow -= f;
                flow += f;
                a -= f;
                if (a == 0) break;
            }
        }
        return flow;
    }

    int Maxflow(int s, int t) 
    {
        this->s = s;
        this->t = t;
        int flow = 0;
        //先分层，判断s->t是否能通
        while (BFS()) 
        {
            memset(cur, 0, sizeof(cur));
            //再找增广路
            flow += DFS(s, INF);//初值设最大，递归的过程中取min
        }
        return flow;//最大流
    }

    int DinicWork(vector<vector<int>>& graph, int n, int s, int t) 
    {
        this->init(n);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graph[i][j] == 0) continue;
                this->AddEdge(i, j, graph[i][j]);
            }
        }
        return this->Maxflow(s, t);
    }
};





#endif
