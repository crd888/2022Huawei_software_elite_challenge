#include "head.h"
#include "Node.h"
#include <set>
#include <cmath>
/*
#define TEST
*/
#ifdef TEST
const char *configFileName = "../data/config.ini";
const char *siteBandwidthFileName = "../data/site_bandwidth.csv";
const char *demandFileName = "../data/demand.csv";
const char *qosFileName = "../data/qos.csv";
const char *outputFileName = "../output/solution.txt";
#else
const char *configFileName = "/data/config.ini";
const char *siteBandwidthFileName = "/data/site_bandwidth.csv";
const char *demandFileName = "/data/demand.csv";
const char *qosFileName = "/data/qos.csv";
const char *outputFileName = "/output/solution.txt";
#endif
using namespace std;
/*****************************************************************************************
Config类
*****************************************************************************************/
Config::Config()
{
    ifstream readFile;
    const char *file;
    file = configFileName;
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open())
    {
        MyPint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[1024];
    char *sub;
    int lineIndex = 1;
    int objIndex = 0;
    bzero(buf, sizeof(buf));
    // 读取文件的第一行，忽略第一行内容
    if (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        lineIndex++;
    }
    else
    {
        MyPint("读取%s文件失败\n", file);
        return;
    }
    // 读取文件的2~N行
    bzero(buf, sizeof(buf));
    int confObjIndex = 0;
    while (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        if ((sub = strstr(buf, qosConstraint)) != NULL)
        {
            int val = 0;
            for (int i = strlen(qosConstraint) + 1; buf[i]; i++)
            {
                val *= 10;
                val += buf[i] - '0';
            }
            constraint = val;
            bzero(buf, sizeof(buf));
        }
    }
    MyPint("文件已读完\n");
}
void Config::showMsg()
{
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|-----------------Show config message------------------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    printf("%16s : %d\n", qosConstraint, constraint);
    //cout << setw(15) << qosConstraint << " : " << constraint << endl;
}
inline int Config::getConstraint()
{
    return constraint;
}

/******************************************************************************************
Edge类
******************************************************************************************/
Edge::Edge()
{
    ifstream readFile;
    const char *file;
    file = siteBandwidthFileName;
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open())
    {
        MyPint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[1024];
    int lineIndex = 1;
    int objIndex = 0;
    bzero(buf, sizeof(buf));
    // 读取文件的第一行
    if (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        lineIndex++;
    }
    else
    {
        MyPint("读取%s文件失败\n", file);
        return;
    }
    // 读取文件的2~N行
    bzero(buf, sizeof(buf));
    while (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        string substr;
        int t = 0;
        for (int i = 0; buf[i] != '\r'; i++)
        {
            if (buf[i] == ',')
            {
                indexEdge[substr] = objIndex;
                name.push_back(substr);
                substr.clear();
                t++;
            }
            else
            {
                substr += buf[i];
            }
        }
        bandwidth.push_back(stoi(substr)); // 读取带宽需求
        lineIndex++;
        objIndex++;
        bzero(buf, sizeof(buf));
    }
    edgeSize = bandwidth.size();
    readFile.close();
    MyPint("文件已读完\n");
}
void Edge::showMsg()
{
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|----------------Show edge node message----------------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    // 输出第一行 ：   name :  bandwidth
    std::cout << std::setw(8) << "index : " << std::setw(8) << "Name : " << std::setw(8) << " Bandwidth" << endl;
    // 输出第2 ~ n行 ： time  name.demand
    for (int i = 0; i < bandwidth.size(); i++)
    {
        printf("%4d  : %4s  : %8d\n", indexEdge[name[i]], name[i].c_str(), bandwidth[i]);
        //cout << std::setw(5) << indexEdge[name[i]] << " : " << std::setw(4) << name[i] << "   :" << std::setw(8) << bandwidth[i] << endl;
    }
}
inline int Edge::size()
{
    return edgeSize;
}
inline void Edge::reStart()
{
    leftBandwidth = bandwidth;
}

/*****************************************************************************************
Client类
******************************************************************************************/
Client::Client()
{
    ifstream readFile;
    const char *file;
    file = demandFileName;
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open())
    {
        MyPint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[1024];
    int lineIndex = 1;
    string substr;
    int index = 0;
    bzero(buf, sizeof(buf));
    // 读取文件的第一行
    if (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        for (int i = 0; buf[i] != '\r'; i++)
        {
            if (buf[i] == ',')
            {
                // 除去mtime,后的部分
                if (index)
                {
                    indexClient[substr] = index - 1;
                    clientNodeList.push_back(ClientNode(index - 1, move(substr)));
                }
                substr.clear();
                index++;
            }
            else
            {
                substr += buf[i];
            }
        }
        indexClient[substr] = index - 1;
        clientNodeList.push_back(ClientNode(index - 1, move(substr)));
        lineIndex++;
    }
    else
    {
        MyPint("读取%s文件失败\n", file);
        return;
    }
    substr.clear();
    // 读取文件的2~N行
    bzero(buf, sizeof(buf));
    while (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        string substr;
        int t = 0;
        for (int i = 0; buf[i] != '\r'; i++)
        {
            if (buf[i] == ',')
            {
                if (t)
                {
                    clientNodeList[t - 1].demand.push_back(stoi(substr)); // 读取带宽需求
                }
                else
                {
                    timeList.push_back(substr); // 读取时间信息
                }
                substr.clear();
                t++;
            }
            else
            {
                substr += buf[i];
            }
        }
        clientNodeList[t - 1].demand.push_back(stoi(substr)); // 读取带宽需求
        lineIndex++;
        bzero(buf, sizeof(buf));
    }
    clientSize = clientNodeList.size();
    readFile.close();
    MyPint("文件已读完\n");
}
void Client::showMsg()
{
    std::cout << "+--------------------------------------------------------+" << endl;
    std::cout << "|----------------Show client node message----------------|" << endl;
    std::cout << "+--------------------------------------------------------+" << endl;
    std::cout << "      Index      :";
    for (int i = 0; i < clientNodeList.size(); i++)
    {
        std::cout << std::setw(8) << indexClient[clientNodeList[i].name];
    }
    std::cout << endl;
    // 输出第一行 ：   time \ Client :  name
    std::cout << "  time \\ Client  :";
    for (int i = 0; i < clientNodeList.size(); i++)
    {
        std::cout << std::setw(8) << clientNodeList[i].name;
    }
    std::cout << endl;
    // 输出第2 ~ n行 ： time  name.demand
    for (int i = 0; i < timeList.size(); i++)
    {
        std::cout << timeList[i] << " :";
        for (int j = 0; j < clientNodeList.size(); j++)
        {
            std::cout << std::setw(8) << clientNodeList[j].demand[i];
        }
        std::cout << endl;
    }
}
inline int Client::size()
{
    return clientSize;
}
void Client::reStart(int k)
{
    for (int i = 0; i < clientSize; i++)
    {
        clientNodeList[i].leftDemand = clientNodeList[i].demand[k];
    }
}

/******************************************************************************************
Solution
******************************************************************************************/
Solution::Solution() : N(edge.size()),
                       M(client.size()),
                       timeLen(client.timeList.size()),
                       constraint(config.getConstraint()),
                       delay(M, vector<int>(N, 0)),
                       access(M, vector<int>(N, 0)),
                       client2Edge(M),
                       edge2Client(N),
                       historyAlloc(N, vector<pair<int, int>>())
{
    ifstream readFile;
    const char *file;
    file = qosFileName;
    fout.open(outputFileName);
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open())
    {
        MyPint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[1024];
    int lineIndex = 1; // 行号
    int objIndex = 0;
    bzero(buf, sizeof(buf));
    vector<int> disorderForEdge(N, 0);
    vector<int> disorderForClient(M, 0);
    // 读取文件的第一行: 第一行为客户节点，为防止无序，使用了disorderForClient对序号重新标定
    if (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        string substr;
        for (int i = 0; buf[i] != '\r'; i++)
        {
            if (buf[i] == ',')
            {
                if (objIndex)
                {
                    disorderForClient[objIndex - 1] = client.indexClient[substr];
                }
                substr.clear();
                objIndex++;
            }
            else
            {
                substr += buf[i];
            }
        }
        disorderForClient[objIndex - 1] = client.indexClient[substr];
        lineIndex++;
    }
    else
    {
        MyPint("读取%s文件失败\n", file);
        return;
    }
    // 读取文件的2~N行: 剩下的为边缘节点，为防止无序，使用了disorderForEdge对序号重新标定
    bzero(buf, sizeof(buf));
    while (readFile.getline(buf, sizeof(buf)))
    {
        MyPint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        string substr;
        int x, y;
        objIndex = 0;
        for (int i = 0; buf[i] != '\r'; i++)
        {
            if (buf[i] == ',')
            {
                if (objIndex)
                { //找到的是延迟
                    x = disorderForEdge[lineIndex - 2], y = disorderForClient[objIndex - 1];
                    delay[y][x] = stoi(substr);
                    if (delay[y][x] >= constraint)
                        access[y][x] = 0;
                    else
                        access[y][x] = 1;
                }
                else
                { //找到的是边缘节点名称
                    disorderForEdge[lineIndex - 2] = edge.indexEdge[substr];
                }
                objIndex++;
                substr.clear();
            }
            else
            {
                substr += buf[i];
            }
        }
        x = disorderForEdge[lineIndex - 2], y = disorderForClient[objIndex - 1];
        delay[y][x] = stoi(substr);
        if (delay[y][x] >= constraint)
            access[y][x] = 0;
        else
            access[y][x] = 1;
        lineIndex++;
        bzero(buf, sizeof(buf));
    }
    readFile.close();
    MyPint("%s文件已读完\n", file);
    /*
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (access[i][j] == 0) continue;
                client2Edge[i].push_back((Client2EdgeMsg){j, delay[i][j]});
            }
        }
        for (int i = 0; i < M; i++) {
            sort(client2Edge[i].begin(), client2Edge[i].end());
        }
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (access[j][i] == 0) continue;
                edge2Client[i].push_back((Client2EdgeMsg){j, delay[j][i]});
            }
        }
        for (int i = 0; i < M; i++) {
            sort(edge2Client[i].begin(), edge2Client[i].end());
        }
        */
    // 以下计算先对哪个客户节点进行分配
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> que1;
    for (int i = 0; i < M; i++)
    {
        int num = 0;
        for (int j = 0; j < N; j++)
        {
            if (access[i][j] == 0)
                continue;
            num++;
        }
        que1.push(pair<int, int>{num, i});
    }
    while (!que1.empty())
    {
        auto x = que1.top();
        que1.pop();
        clientGetSequence.push_back(x.second);
        // 边缘节点i能使用的客户节点，按客户节点能分配对象数量的个数从小到大排序
        for (int i = 0; i < N; i++)
        {
            if (access[x.second][i] == 0)
                continue;
            edge2Client[i].push_back(x.second);
        }
    }
    // 以下计算先对哪个边缘节点进行分配
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> que2;
    for (int i = 0; i < N; i++)
    {
        int num = 0;
        for (int j = 0; j < M; j++)
        {
            if (access[j][i] == 0)
                continue;
            num++;
        }
        que2.push(pair<int, int>{num, i});
    }
    while (!que2.empty())
    {
        auto x = que2.top();
        que2.pop();
        if (x.first == 0)
            continue;
        edgeAllocationSequence.push_back(x.second);
        // 客户节点i能使用的边缘节点，按边缘节点能分配对象数量的个数从小到大排序
        for (int i = 0; i < M; i++)
        {
            if (access[i][x.second] == 0)
                continue;
            client2Edge[i].push_back(x.second);
        }
    }
}
Solution::~Solution()
{
    fout.close();
#ifdef TEST
    int index = (int)ceil(timeLen * 0.95) - 1;
    //if (historyAlloc[0].size() != timeLen) return;
    fout.open("../output/data.txt");
    fout << "       ";
    for (int i = 0; i < N; i++)
    {
        fout << setw(7) << edge.name[i];
    }
    fout << endl;
    for (int k = 0; k < timeLen; k++)
    {
        fout << setw(4) << k << " : ";
        for (int i = 0; i < N; i++)
        {
            fout << setw(7) << historyAlloc[i][k].first;
        }
        fout << endl;
    }
    fout.close();

    unsigned long long grade = 0;
    for (int i = 0; i < N; i++)
    {
        sort(historyAlloc[i].begin(), historyAlloc[i].end());
        grade += historyAlloc[i][index].first;
    }
    std::cout << "此次得分为 : " << grade << endl;
    fout.open("../output/dataSort.txt");
    fout << "       ";
    for (int i = 0; i < N; i++)
    {
        fout << setw(13) << edge.name[i];
    }
    fout << endl;
    for (int k = 0; k < timeLen; k++)
    {

        fout << setw(4) << k << " : ";
        for (int i = 0; i < N; i++)
        {
            fout << setw(7) << historyAlloc[i][k].first << "(" << setw(4) << historyAlloc[i][k].second << ")";
        }
        fout << endl;
    }
    fout.close();
#endif
}
void Solution::showBaseMatrixMsg()
{
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|---------Show Edge2ClientDelay BaseMatrixMsg----------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "edgeName \\ clientName : ";
    for (int i = 0; i < M; i++)
    {
        printf("%7s  ", client.clientNodeList[i].name.c_str());
    }
    std::cout << endl;
    for (int i = 0; i < N; i++)
    {
        printf("%-10d -> %7s : ", i + 1, edge.name[i].c_str());
        for (int j = 0; j < M; j++)
        {
            //printf("(%d)%4d  ", access[j][i], delay[j][i]);
            printf("%d,  ", access[j][i]);
        }
        printf("\n");
    }
}
void Solution::showClientCanUse()
{
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|---------index : ClientName Can use EdgeMsg-----------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    for (int i = 0; i < M; i++)
    {
        i > 0 && printf(" -> ");
        printf("%2s", edge.name[clientGetSequence[i]].c_str());
    }
    printf(".\n");
    for (int i = 0; i < M; i++)
    {
        printf("%4d -> %2s : ", i, client.clientNodeList[i].name.c_str());
        for (int j = 0; j < client2Edge[i].size(); j++)
        {
            printf("<%2s,%4d>  ", edge.name[client2Edge[i][j]].c_str(), delay[i][client2Edge[i][j]]);
        }
        printf("\n");
    }
}
void Solution::showEdgeCanUse()
{
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|----------index : EdgeName Can use EdgeMsg------------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    for (int i = 0; i < N; i++)
    {
        i > 0 && printf(" -> ");
        printf("%2s", edge.name[edgeAllocationSequence[i]].c_str());
    }
    printf(".\n");
    for (int i = 0; i < N; i++)
    {
        printf("%4d -> %2s : ", i, edge.name[i].c_str());
        for (int j = 0; j < edge2Client[i].size(); j++)
        {
            printf("<%-2s,%4d>  ", client.clientNodeList[edge2Client[i][j]].name.c_str(), delay[edge2Client[i][j]][i]);
        }
        printf("\n");
    }
}
/*
    输出函数
*/
void Solution::outputSolution(vector<vector<int>> &allot, int k)
{
    for (int i = 0; i < M; i++)
    {
        int mark = 0;
        fout << client.clientNodeList[i].name << ":";
        for (int j = 0; j < N; j++)
        {
            if (allot[i][j] == 0)
                continue;
            if (mark == 1)
                fout << ",";
            fout << "<" << edge.name[j] << "," << allot[i][j] << ">";
            mark = 1;
        }
        fout << endl;
    }
    int ksum = 0;
    for (int i = 0; i < M; i++)
    {
        int sum = 0;
        for (int j = 0; j < N; j++)
        {
            if (allot[i][j] > 0 && access[i][j] == 0)
                std::cout << "分配错误2" << endl;
            sum += allot[i][j];
        }
        if (sum != client.clientNodeList[i].demand[k])
        {
            std::cout << k << "时刻分配错误1, " << client.clientNodeList[i].name << "需求量为 : " << client.clientNodeList[i].demand[k] << ", 为其分配了" << sum << endl;
        }
        ksum += sum;
    }
    //std::cout << k << " : " << ksum << endl;

    for (int j = 0; j < N; j++)
    {
        int sum = 0;
        for (int i = 0; i < M; i++)
        {
            if (allot[i][j] > 0 && access[i][j] == 0)
                std::cout << k << "时刻分配错误2 : 使用了超延迟的带宽" << endl;
            sum += allot[i][j];
        }
        if (sum > edge.bandwidth[j])
        {
            std::cout << k << "时刻分配错误3 : 边缘节点提供带宽超过上限, " << edge.name[j] << "上限为" << edge.bandwidth[j] << ", 其为客户节点分配了共" << sum << endl;
        }
        //cout << edge.name[j] << "分配的总带宽为" << sum << endl;
        historyAlloc[j].push_back(pair<int, int>{sum, k});
    }
}
/*
    分配最大的5%值
*/
void Solution::allocBiggest5_1(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int biggerNum)
{
    //for (int y : edgeAllocationSequence) {  // 选择哪个边缘节点先进行分配
    for (int ttt = edgeAllocationSequence.size() - 1; ttt > -1; ttt--)
    {
        int y = edgeAllocationSequence[ttt];
        // 用于找biggerNum个最大能分配的时刻k，k时刻的总需求越大越先进行分配
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
        for (int k = 0; k < timeLen; k++)
        {
            int sum = 0;
            for (int x : edge2Client[y])
            {                                  // 计算边缘节点y所能分配的客户节点在k时刻的总带宽需求
                sum += clientLeftDemand[k][x]; // 将客户还需要的需求带宽加起来
            }
            queEdge.push(pair<int, int>{sum, k});
        }
        // 选取biggerNum个最大时刻进行分配
        for (int i = 0; i < biggerNum; i++)
        {
            pair<int, int> temp = queEdge.top();
            queEdge.pop();
            int needBandwidth = temp.first, k = temp.second;
            //cout << "->" << needBandwidth << endl;
            int allocBandwidth = min(needBandwidth, edgeLeftBandwidth[k][y]); // 计算需要分配的带宽
            // 如何分配那么多带宽?
            // 方案一：按照这些客户中在k时刻能使用总带宽值最少的开始进行分配,每次都将客户端的所有带宽需求满足
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> queClient;
            for (int x : edge2Client[y])
            {
                // 没有需求，不进队列
                if (clientLeftDemand[k][x] == 0)
                    continue;
                int sum = 0;
                for (int y1 : client2Edge[x])
                {
                    sum += edgeLeftBandwidth[k][y1];
                }
                queClient.push(pair<int, int>{sum, x});
            }
            while (!queClient.empty() && edgeLeftBandwidth[k][y] > 0)
            {
                pair<int, int> temp2 = queClient.top();
                queClient.pop();
                // 获得操作客户对象
                int clientCanUseLeftBandwidth = temp2.first, x = temp2.second;
                // 计算需求并进行分配
                int demand = clientLeftDemand[k][x];
                if (demand > edgeLeftBandwidth[k][y])
                    demand = edgeLeftBandwidth[k][y];
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                clientLeftDemand[k][x] -= demand;
                allot[k][x][y] += demand;
                // 将数据弹出后重新分配
                while (!queClient.empty())
                    queClient.pop();
                for (int x : edge2Client[y])
                {
                    // 没有需求，不进队列
                    if (clientLeftDemand[k][x] == 0)
                        continue;
                    int sum = 0;
                    for (int y1 : client2Edge[x])
                        sum += edgeLeftBandwidth[k][y1];
                    queClient.push(pair<int, int>{sum, x});
                }
            }
        }
    }
}

//5%分配选择：根据客户需求，还是根据边缘剩余带宽定顺序？？？？？？？？
void Solution::allocBiggest5_2(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int biggerNum)
{
    // 用于找biggerNum个最大能分配的时刻k，k时刻的总需求越大越先进行分配
    priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
    for (int y = 0; y < N; y++)
    {
        queEdge.push(pair<int, int>(edge.bandwidth[y], y));
    }
    while (!queEdge.empty())
    { // 选择带宽大的边缘节点先进行分配
        pair<int, int> temp = queEdge.top();
        int y = temp.second;
        queEdge.pop();
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        for (int k = 0; k < timeLen; k++)
        {
            int sum = 0;
            for (int x : edge2Client[y])
            {
                sum += clientLeftDemand[k][x];
            }
            queTime.push(pair<int, int>(sum, k));
        }

        for (int i = 0; i < biggerNum; i++) // 选择总带宽需求高的时刻先进行分配
        {
            pair<int, int> temp2 = queTime.top();
            queTime.pop();
            int k = temp2.second;
            //5%分配选择：根据客户需求，还是根据边缘剩余带宽定顺序
            priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queClient;
            for (int x : edge2Client[y])
            {
                if (clientLeftDemand[k][x] == 0)
                    continue;
                queClient.push(pair<int, int>(clientLeftDemand[k][x], x));

                /*if (clientLeftDemand[k][x] == 0) continue;
                int sum = 0;
                for (int y1 : client2Edge[x]) {
                    sum += edgeLeftBandwidth[k][y1];
                }
                queClient.push(pair<int, int>{sum, x});
                */
            }

            while (!queClient.empty() && edgeLeftBandwidth[k][y] > 0)
            { // 选择在k时刻带宽需求高的客户节点先进行分配
                pair<int, int> temp3 = queClient.top();
                queClient.pop();
                int x = temp3.second;
                int demand = clientLeftDemand[k][x];
                if (demand > edgeLeftBandwidth[k][y])
                    demand = edgeLeftBandwidth[k][y];
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                clientLeftDemand[k][x] -= demand;
                allot[k][x][y] += demand;

                while (!queClient.empty())
                    queClient.pop();
                for (int x : edge2Client[y])
                {
                    /*
                    if (clientLeftDemand[k][x] == 0)
                        continue;
                    queClient.push(pair<int, int>(clientLeftDemand[k][x], x));
*/
                    if (clientLeftDemand[k][x] == 0) continue;
                    int sum = 0;
                    for (int y1 : client2Edge[x]) {
                        sum += edgeLeftBandwidth[k][y1];
                    }
                    queClient.push(pair<int, int>{sum, x});
                    
                }
            }
        }
    }
}

void Solution::allocBiggest5_6(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int biggerNum)
{
     srand(time(NULL));
     //随机选择一个边缘分配
    int nums = edgeAllocationSequence.size();
    int ram = rand() % nums;
    int y = edgeAllocationSequence[ram];
    int update = 0;
   
   
    for (int j = 0; j < N; j++)
    { 
        if (update == 1)
        {
            int nums = edgeAllocationSequence.size();
            ram = rand() % nums;
            y = edgeAllocationSequence[ram];
        }
        cout << "随机边缘：" << y << endl;
        update = 1;

        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        for (int k = 0; k < timeLen; k++)
        {
            int sum = 0;
            for (int x : edge2Client[y])
            {
                sum += clientLeftDemand[k][x];
            }
            queTime.push(pair<int, int>(sum, k));
        }

        for (int i = 0; i < biggerNum; i++) // 选择总带宽需求高的时刻先进行分配
        {
            pair<int, int> temp2 = queTime.top();
            queTime.pop();
            int k = temp2.second;
            //5%分配:  根据边缘剩余带宽定顺序
            priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queClient;
            for (int x : edge2Client[y])
            {
                if (clientLeftDemand[k][x] == 0)
                    continue;
                queClient.push(pair<int, int>(clientLeftDemand[k][x], x));
            }

            while (!queClient.empty() && edgeLeftBandwidth[k][y] > 0)
            { // 选择在k时刻带宽需求高的客户节点先进行分配
                pair<int, int> temp3 = queClient.top();
                queClient.pop();
                int x = temp3.second;
                int demand = clientLeftDemand[k][x];
                if (demand > edgeLeftBandwidth[k][y])
                    demand = edgeLeftBandwidth[k][y];
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                clientLeftDemand[k][x] -= demand;
                allot[k][x][y] += demand;

                while (!queClient.empty())
                    queClient.pop();
                for (int x : edge2Client[y])
                {
                    if (clientLeftDemand[k][x] == 0)
                        continue;
                    queClient.push(pair<int, int>(clientLeftDemand[k][x], x));
                }
            }
        }
    }
}



void Solution::allocBiggest5_5(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int biggerNum)
{
    srand(time(NULL));

    priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
    for (int k = 0; k < timeLen; k++)
    {
        int sum = 0;
        for (size_t x = 0; x < M; x++)
        {
            sum += clientLeftDemand[k][x];
        }
        queTime.push(pair<int, int>(sum, k));
    }

    for (int i = 0; i < biggerNum; i++) // 选择总带宽需求高的时刻先进行分配
    {
        pair<int, int> temp2 = queTime.top();
        queTime.pop();
        int k = temp2.second;

        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queClient;
        for (size_t x = 0; x < M; x++)
        {
            if (clientLeftDemand[k][x] == 0)
                continue;
            queClient.push(pair<int, int>(clientLeftDemand[k][x], x));
        }
        pair<int, int> temp3 = queClient.top();
        queClient.pop();
        int x = temp3.second;
        //随机选择一个边缘分配
        int nums = client2Edge[x].size();
        int ram = rand() % nums;
        int y = client2Edge[x][ram];
        int update = 0;

        //随即选择边缘分配
        while (!queClient.empty() && edgeLeftBandwidth[k][y] > 0)
        { // 选择在k时刻带宽需求高的客户节点先进行分配
            if (update == 1)
            {
                pair<int, int> temp3 = queClient.top();
                queClient.pop();
                int x = temp3.second;
                //随机选择一个边缘分配
                int nums = client2Edge[x].size();
                ram = rand() % nums;
                y = client2Edge[x][ram];
            }
            cout << "随机边缘：" << y << endl;
            update = 1;
            int demand = clientLeftDemand[k][x];

            if (demand > edgeLeftBandwidth[k][y])
                demand = edgeLeftBandwidth[k][y];
            edgeLeftBandwidth[k][y] -= demand;
            edgeUsedBandwidth[k][y] += demand;
            clientLeftDemand[k][x] -= demand;
            allot[k][x][y] += demand;

            while (!queClient.empty())
                queClient.pop();
            for (int x : edge2Client[y])
            {
                if (clientLeftDemand[k][x] == 0)
                    continue;
                queClient.push(pair<int, int>(clientLeftDemand[k][x], x));

                /*if (clientLeftDemand[k][x] == 0) continue;
                    int sum = 0;
                    for (int y1 : client2Edge[x]) {
                        sum += edgeLeftBandwidth[k][y1];
                    }
                    queClient.push(pair<int, int>{sum, x});
                    */
            }
        }
    }
}

void Solution::allocBiggest5_4(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int biggerNum)
{
    // 用于找biggerNum个最大能分配的时刻k，k时刻的总需求越大越先进行分配
    priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
    for (int y = 0; y < N; y++)
    {
        int sum = 0;
        for (int i = 0; i < M; i++)
        {
            if (access[i][y] == 1)
            {
                sum++;
            }
        }
        queEdge.push(pair<int, int>(sum, y));
    }
    while (!queEdge.empty())
    { // 选择带宽大的边缘节点先进行分配
        pair<int, int> temp = queEdge.top();
        int y = temp.second;
        queEdge.pop();
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        for (int k = 0; k < timeLen; k++)
        {
            int sum = 0;
            for (int x : edge2Client[y])
            {
                sum += clientLeftDemand[k][x];
            }
            queTime.push(pair<int, int>(sum, k));
        }

        for (int i = 0; i < biggerNum; i++) // 选择总带宽需求高的时刻先进行分配
        {
            pair<int, int> temp2 = queTime.top();
            queTime.pop();
            int k = temp2.second;
            //5%分配选择：根据客户需求，还是根据边缘剩余带宽定顺序？？？？？？？？
            priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queClient;
            for (int x : edge2Client[y])
            {
                if (clientLeftDemand[k][x] == 0)
                    continue;
                queClient.push(pair<int, int>(clientLeftDemand[k][x], x));

                /*if (clientLeftDemand[k][x] == 0) continue;
                int sum = 0;
                for (int y1 : client2Edge[x]) {
                    sum += edgeLeftBandwidth[k][y1];
                }
                queClient.push(pair<int, int>{sum, x});
                */
            }

            while (!queClient.empty() && edgeLeftBandwidth[k][y] > 0)
            { // 选择在k时刻带宽需求高的客户节点先进行分配
                pair<int, int> temp3 = queClient.top();
                queClient.pop();
                int x = temp3.second;
                int demand = clientLeftDemand[k][x];
                if (demand > edgeLeftBandwidth[k][y])
                    demand = edgeLeftBandwidth[k][y];
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                clientLeftDemand[k][x] -= demand;
                allot[k][x][y] += demand;

                while (!queClient.empty())
                    queClient.pop();
                for (int x : edge2Client[y])
                {
                    if (clientLeftDemand[k][x] == 0)
                        continue;
                    queClient.push(pair<int, int>(clientLeftDemand[k][x], x));

                    /*if (clientLeftDemand[k][x] == 0) continue;
                    int sum = 0;
                    for (int y1 : client2Edge[x]) {
                        sum += edgeLeftBandwidth[k][y1];
                    }
                    queClient.push(pair<int, int>{sum, x});
                    */
                }
            }
        }
    }
}

void Solution::allocBiggest5_3(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int biggerNum)
{
    int n = N + M + 2;
    // 用于找biggerNum个最大能分配的时刻k，k时刻的总需求越大越先进行分配
    priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
    for (int y = 0; y < N; y++)
    {
        queEdge.push(pair<int, int>(edge.bandwidth[y], y));
    }
    for (int y : edgeAllocationSequence)
    { // 选择带宽大的边缘节点先进行分配
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        for (int k = 0; k < timeLen; k++)
        {
            int sum = 0;
            for (int x : edge2Client[y])
            {
                sum += clientLeftDemand[k][x];
            }
            queTime.push(pair<int, int>(sum, k));
        }

        for (int i = 0; i < biggerNum; i++) // 选择总带宽需求高的时刻先进行分配
        {
            pair<int, int> temp2 = queTime.top();
            queTime.pop();
            int k = temp2.second;

            vector<int> reAlloc(N, 0);
            for (int i = 0; i < N; i++)
            {
                //统计时刻k的使用量
                reAlloc[i] = edgeLeftBandwidth[k][i];
            }
            // 这一时刻的带宽总需求
            int sumDemand = 0;
            for (int i = 0; i < M; i++)
            {
                sumDemand += client.clientNodeList[i].demand[k];
            }

            //使用Dinic算法尝试进行分配
            Dinic sol;
            vector<vector<int>> graph(n, vector<int>(n, 0));
            sol.init(n);
            for (int i = 0; i < N; i++)
            {
                graph[0][i + 1] = reAlloc[i];
            }
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < M; j++)
                {
                    if (access[j][i] == 0)
                        continue;
                    graph[i + 1][j + N + 1] = 0x7fffffff;
                }
            }
            for (int i = 0; i < M; i++)
            {
                if (client.clientNodeList[i].demand[k] == 0)
                    continue;
                graph[i + N + 1][N + M + 1] = client.clientNodeList[i].demand[k];
            }
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    if (graph[i][j] == 0)
                        continue;
                    sol.AddEdge(i, j, graph[i][j]);
                }
            }
            int ret = sol.Maxflow(0, n - 1);
            if (ret != sumDemand)
            {
                std::cout << "分配失败" << endl;
                break;
            }
            cout << "5%已分配完" << i << "次" << endl;

            for (int i = 0; i < sol.edges.size(); i++)
            {
                int from = sol.edges[i].from, to = sol.edges[i].to, val = sol.edges[i].flow;
                if (from == 0 && to >= 1 && to <= N)
                { // 边缘节点分配出去的带宽和
                    int y1 = to - 1;
                    edgeLeftBandwidth[k][y1] = edge.bandwidth[i] - val;
                    edgeUsedBandwidth[k][y1] = val;
                }
                else if (from >= 1 && from <= N && to >= N + 1 && to <= N + M)
                {
                    int x1 = to - N - 1, y1 = from - 1;
                    allot[k][x1][y1] = val;
                }
                else if (from >= N + 1 && from <= N + M && to == N + M + 1)
                {
                    int x1 = from - N - 1;
                    clientLeftDemand[k][x1] = client.clientNodeList[x1].demand[k] - val;
                }
            }
        }
    }
}

//分配函数:根据需求进行平均分
void Solution::doAllocOptByAveDemand(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand)
{
    for (int k = 0; k < timeLen; k++)
    {
        int n = N + M + 2;
        int sumDemand = 0;
        vector<vector<int>> graph(n, vector<int>(n, 0));
        vector<int> edgeWantAlloc(N, 0);
        vector<int> edgeWantAlloc1(N, 0);
        int flag = 1;
        int numEdge = 0;
        Dinic sol;

        //当前时间序列的总需求
        for (int i = 0; i < M; i++)
        {
            sumDemand += clientLeftDemand[k][i];
        }
        for (int y : edgeAllocationSequence)
        {
            if (edgeLeftBandwidth[k][y] == 0)
                continue;
            numEdge++;
        }
        int aveDemand = sumDemand / numEdge;

        for (int y : edgeAllocationSequence)
        {
            edgeWantAlloc[y] = min(aveDemand, edgeLeftBandwidth[k][y]);
        }
        /*
        int aveDemand1 = 0;
        for (int j = 0; j < N; j++)
        {
            int sum = 0;
            for (size_t i = 0; i < M; i++)
            {
                 if (edgeLeftBandwidth[k][j] == 0 || access[i][j] == 0)
                    continue;
                 sum++;
            }
            //边缘j应该给每个客户i分配多少
            aveDemand1 = edgeLeftBandwidth[k][j] / sum;
            edgeWantAlloc[j] = min(aveDemand1, edgeLeftBandwidth[k][j]);
        }
*/
        // 对方案进行尝试分配
        while (flag)
        {
            //************构建图*************
            for (int i = 0; i < N; i++)
            {
                graph[0][i + 1] = edgeWantAlloc[i];
            }
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < M; j++)
                {
                    if (access[j][i] == 0)
                        continue;
                    graph[i + 1][j + N + 1] = 0x7fffffff;
                }
            }
            for (int i = 0; i < M; i++)
            {
                if (clientLeftDemand[k][i] == 0)
                    continue;
                graph[i + N + 1][N + M + 1] = clientLeftDemand[k][i];
            }

            //************计算最大流***************
            sol.init(n);
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    if (graph[i][j] == 0)
                        continue;
                    sol.AddEdge(i, j, graph[i][j]);
                }
            }

            int ret = sol.Maxflow(0, n - 1);
            //*******************************

            if (ret == sumDemand)
                flag = 0;
            else
            {
                aveDemand += ((sumDemand - ret) / numEdge) + 1;
                for (int y : edgeAllocationSequence)
                {
                    edgeWantAlloc[y] = min(aveDemand, edgeLeftBandwidth[k][y]);
                }
                /*
                for (int j = 0; j < N; j++)
                {
                    int sum = 0;
                    for (size_t i = 0; i < M; i++)
                    {
                        if (edgeLeftBandwidth[k][j] == 0 || access[i][j] == 0)
                            continue;
                        sum++;
                    }
                    //边缘j应该给每个客户i分配多少
                    aveDemand += ((sumDemand - ret) / sum) + 1;
                    edgeWantAlloc1[j] = min(aveDemand, edgeLeftBandwidth[k][j]);
                }
                */
            }
        }

        //*********** 通过最大流分配成功后，对数据进行保存***********
        for (int i = 0; i < sol.edges.size(); i++)
        {
            int from = sol.edges[i].from;
            int to = sol.edges[i].to;
            int val = sol.edges[i].flow;
            if (from == 0 && to >= 1 && to <= N)
            { // 边缘节点分配出去的带宽和
                int y = to - 1;
                edgeLeftBandwidth[k][y] -= val;
                edgeUsedBandwidth[k][y] += val;
            }
            else if (from >= 1 && from <= N && to >= N + 1 && to <= N + M)
            {
                int x = to - N - 1, y = from - 1;
                allot[k][x][y] += val;
            }
            else if (from >= N + 1 && from <= N + M && to == N + M + 1)
            {
                int x = from - N - 1;
                clientLeftDemand[k][x] -= val;
            }
        }
    }
}
// 3月20 新思路：在5%时刻分配完后，其他的时刻的数值应该尽可能的平均分，不要出现0的情况
void Solution::allocFunc()
{
    // 所有时刻的分配方案
    vector<vector<vector<int>>> allot(timeLen, vector<vector<int>>(M, vector<int>(N, 0)));
    // 边缘节点在不同时刻的剩余带宽
    vector<vector<int>> edgeLeftBandwidth(timeLen, vector<int>(N));
    // 边缘节点在不同时刻的使用掉的带宽
    vector<vector<int>> edgeUsedBandwidth(timeLen, vector<int>(N, 0));
    // 客户节点在不同时刻的剩余需求
    vector<vector<int>> clientLeftDemand(timeLen, vector<int>(M));
    // 95%的序列的下标
    int index = (int)ceil(timeLen * 0.95) - 1;
    int biggerNum = timeLen - index - 1; // 每个客户节点有biggerIndexNum次来尽最大能力的进行分配
    cout << "index = " << index << ", num = " << biggerNum << endl;
    cout << "M = " << M << ", N = " << N << endl;
    // 分配信息初始化
    for (int k = 0; k < timeLen; k++)
    {
        for (int i = 0; i < N; i++)
            edgeLeftBandwidth[k][i] = edge.bandwidth[i];
        for (int i = 0; i < M; i++)
            clientLeftDemand[k][i] = client.clientNodeList[i].demand[k];
    }

    /*******************************************************************************************************
    此处要好好优化一下
    ********************************************************************************************************/
    // 先分配5%
    allocBiggest5_2(allot, edgeLeftBandwidth, edgeUsedBandwidth, clientLeftDemand, biggerNum); // 50w分数的方法
    //allocBiggest_48w(allot, edgeLeftBandwidth, edgeUsedBandwidth, clientLeftDemand, biggerNum);      // 50w分数的方法
    int sumVal = 0, mk = 0;
    for (int k = 0; k < timeLen; k++)
    {
        int val = 0;
        for (int i = 0; i < M; i++)
        {
            val += clientLeftDemand[k][i];
        }
        if (sumVal < val)
        {
            sumVal = val;
            mk = k;
        }
    }
    cout << "剩下所有时刻中" << mk << "时刻客户需求带宽最大，为" << sumVal << endl;

    vector<priority_queue<int, vector<int>, greater<int>>> biggest(N, priority_queue<int, vector<int>, greater<int>>());
    for (int y : edgeAllocationSequence)
    {
        //biggest[y].push(0);
        for (int k = 0; k < timeLen; k++)
        {
            if (edgeLeftBandwidth[k][y] < edge.bandwidth[y])
            {
                //cout << "<" << k << ", " << edgeUsedBandwidth[k][y] << ">, ";
                biggest[y].push(edgeUsedBandwidth[k][y]);
            }
        }
    }
    // 使用Dinic算法对每一个时刻的信息进行分配，要求尽可能满足当前分配的值小于等于biggest[y].top();
    doAllocOptByAveDemand(allot, edgeLeftBandwidth, edgeUsedBandwidth, clientLeftDemand);
    //doAllocOpt_48w(allot, edgeLeftBandwidth, edgeUsedBandwidth, clientLeftDemand);
    // 写对最后一行的优化：以时刻最多者为参考，时刻不符合的将自己的带宽分配出去(不断分配，直至不能再进行分配)
    reAllocFunc(allot, edgeLeftBandwidth, edgeUsedBandwidth, clientLeftDemand, index);
    for (int k = 0; k < timeLen; k++)
    {
        outputSolution(allot[k], k);
    }
}
void Solution::showClientDemand()
{
}
void Solution::startWork()
{
    allocFunc();
    //getColor();
}

void Solution::reAllocFunc(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int index)
{
    int n = N + M + 2;
    //int y = N - 1;
    for (int y : edgeAllocationSequence)
    {
        while (1)
        {
            //cout << y << "-> 1: 寻找各数据95%的位置" << endl; //index位置

            //边缘i在时刻k下的使用量
            vector<vector<pair<int, int>>> edgeSortVal(N, vector<pair<int, int>>(timeLen));
            for (int i = 0; i < N; i++)
            {
                for (int k = 0; k < timeLen; k++)
                {
                    edgeSortVal[i][k] = pair<int, int>(edgeUsedBandwidth[k][i], k);
                }
                sort(edgeSortVal[i].begin(), edgeSortVal[i].end());
            }
            //边缘y在95%时刻k的使用量
            int usedBandwidth = edgeSortVal[y][index].first;
            int k = edgeSortVal[y][index].second;

            if (usedBandwidth == 0)
                break;

            vector<int> reAlloc(N, 0); // 新的分配方案
            for (int i = 0; i < N; i++)
            {
                reAlloc[i] = edgeUsedBandwidth[k][i]; //统计95%时刻的使用量
            }
            int sumDemand = 0; // 这一时刻的带宽总需求
            for (int i = 0; i < M; i++)
            {
                sumDemand += client.clientNodeList[i].demand[k];
            }

            //std::cout << y << "的95%位置使用了时刻" << k << ", 值为" << usedBandwidth << endl;

            //cout << y << "-> 2: 设置重新分配的值" << endl;
            int canImprove = 0;
            for (int i = 0; i < N; i++)
            {
                if (reAlloc[y] == 0)
                    break;
                if (i == y)
                    continue;
                if (edgeUsedBandwidth[k][i] == edge.bandwidth[i])
                    continue;
                if (edgeUsedBandwidth[k][i] == edgeSortVal[i][index].first)
                    continue;

                // k时刻当前节点使用量为5%区域以内的
                if (edgeUsedBandwidth[k][i] > edgeSortVal[i][index].first)
                {
                    //边缘i还可以使用的
                    int err = edge.bandwidth[i] - edgeSortVal[i][index].first;
                    canImprove += err;
                    reAlloc[i] += err;
                    reAlloc[y] = max(reAlloc[y] - err, 0);//边缘y再分配给i
                }
                else// k时刻当前节点
                {
                    int err = edgeSortVal[i][index].first - edgeUsedBandwidth[k][i];//想要优化的量
                    if (err > edge.bandwidth[i] - reAlloc[i])
                        err = edge.bandwidth[i] - reAlloc[i];
                    if (reAlloc[y] < err)
                        err = reAlloc[y];
                    canImprove += err;
                    reAlloc[i] += err;
                    reAlloc[y] -= err;
                }
            }
            if (canImprove == 0)
                break;
            // 3: 使用Dinic算法尝试进行分配
            //cout << y << "-> 3: 使用Dinic算法尝试进行分配, 希望提升" << canImprove << endl;
            Dinic sol;
            vector<vector<int>> graph(n, vector<int>(n, 0));
            sol.init(n);
            for (int i = 0; i < N; i++)
            {
                graph[0][i + 1] = reAlloc[i];
            }
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < M; j++)
                {
                    if (access[j][i] == 0)
                        continue;
                    graph[i + 1][j + N + 1] = max(reAlloc[i], client.clientNodeList[j].demand[k]);
                }
            }
            for (int i = 0; i < M; i++)
            {
                if (client.clientNodeList[i].demand[k] == 0)
                    continue;
                graph[i + N + 1][N + M + 1] = client.clientNodeList[i].demand[k];
            }
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    if (graph[i][j] == 0)
                        continue;
                    sol.AddEdge(i, j, graph[i][j]);
                }
            }
            int ret = sol.Maxflow(0, n - 1);
            if (ret != sumDemand)
            {
                std::cout << "分配失败" << endl;
                break;
            }
            for (int i = 0; i < sol.edges.size(); i++)
            {
                int from = sol.edges[i].from, to = sol.edges[i].to, val = sol.edges[i].flow;
                if (from == 0 && to >= 1 && to <= N)
                { // 边缘节点分配出去的带宽和
                    int y1 = to - 1;
                    edgeLeftBandwidth[k][y1] = edge.bandwidth[i] - val;
                    edgeUsedBandwidth[k][y1] = val;
                }
                else if (from >= 1 && from <= N && to >= N + 1 && to <= N + M)
                {
                    int x1 = to - N - 1, y1 = from - 1;
                    allot[k][x1][y1] = val;
                }
                else if (from >= N + 1 && from <= N + M && to == N + M + 1)
                {
                    int x1 = from - N - 1;
                    clientLeftDemand[k][x1] = client.clientNodeList[x1].demand[k] - val;
                }
            }
            if (usedBandwidth == edgeUsedBandwidth[k][y])
                break;
            //std::cout << "分配完成, 现在" << y << "的" << k << "时刻的值为" << edgeUsedBandwidth[k][y] << ", 降低了" << usedBandwidth - edgeUsedBandwidth[k][y] << endl;
        }
        //break;
    }
}
/*
    历史方案
*/
void Solution::allocBiggest_48w(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand, int biggerNum)
{
    for (int y : edgeAllocationSequence)
    {
        // 用于找biggerNum个最大能分配的时刻k，k时刻的总需求越大越先进行分配
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
        for (int k = 0; k < timeLen; k++)
        {
            int sum = 0;
            for (int x : edge2Client[y])
            {                                  // 计算边缘节点y所能分配的客户节点在k时刻的总带宽需求
                sum += clientLeftDemand[k][x]; // 将客户还需要的需求带宽加起来
            }
            queEdge.push(pair<int, int>{sum, k});
        }
        // 选取biggerNum个最大时刻进行分配
        for (int i = 0; i < biggerNum; i++)
        {
            pair<int, int> temp = queEdge.top();
            queEdge.pop();
            int needBandwidth = temp.first, k = temp.second;
            //cout << "->" << needBandwidth << endl;
            int allocBandwidth = min(needBandwidth, edgeLeftBandwidth[k][y]); // 计算需要分配的带宽
            // 如何分配那么多带宽?
            // 方案一：按照这些客户中在k时刻能使用总带宽值最少的开始进行分配,每次都将客户端的所有带宽需求满足
            priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queClient;
            for (int x : edge2Client[y])
            {
                // 没有需求，不进队列
                if (clientLeftDemand[k][x] == 0)
                    continue;
                int sum = clientLeftDemand[k][x];
                queClient.push(pair<int, int>{sum, x});
            }
            while (!queClient.empty() && edgeLeftBandwidth[k][y] > 0)
            {
                pair<int, int> temp2 = queClient.top();
                queClient.pop();
                // 获得操作客户对象
                int clientCanUseLeftBandwidth = temp2.first, x = temp2.second;
                // 计算需求并进行分配
                int demand = clientLeftDemand[k][x];
                if (demand > edgeLeftBandwidth[k][y])
                    demand = edgeLeftBandwidth[k][y];
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                clientLeftDemand[k][x] -= demand;
                allot[k][x][y] += demand;
                // 将数据弹出后重新分配
                while (!queClient.empty())
                    queClient.pop();
                for (int x : edge2Client[y])
                {
                    // 没有需求，不进队列
                    if (clientLeftDemand[k][x] == 0)
                        continue;
                    int sum = clientLeftDemand[k][x];
                    queClient.push(pair<int, int>{sum, x});
                }
            }
        }
    }
}
void Solution::doAllocOpt_48w(vector<vector<vector<int>>> &allot, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &clientLeftDemand)
{
    for (int k = 0; k < timeLen; k++)
    {
        int n = N + M + 2;
        int sumDemand = 0;
        vector<vector<int>> graph(n, vector<int>(n, 0));
        vector<int> edgeWantAlloc(N, 0);
        int flag = 1;
        Dinic sol;
        for (int i = 0; i < M; i++)
        {
            sumDemand += clientLeftDemand[k][i];
        }
        int aveDemand = sumDemand / edgeAllocationSequence.size();
        for (int y : edgeAllocationSequence)
        {
            edgeWantAlloc[y] = min(aveDemand, edgeLeftBandwidth[k][y]);
        }
        // 对方案进行尝试分配
        while (flag)
        {
            for (int i = 0; i < N; i++)
            {
                graph[0][i + 1] = edgeWantAlloc[i];
            }
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < M; j++)
                {
                    if (access[j][i] == 0)
                        continue;
                    graph[i + 1][j + N + 1] = 0x7fffffff;
                }
            }
            for (int i = 0; i < M; i++)
            {
                if (clientLeftDemand[k][i] == 0)
                    continue;
                graph[i + N + 1][N + M + 1] = clientLeftDemand[k][i];
            }
            sol.init(n);
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    if (graph[i][j] == 0)
                        continue;
                    sol.AddEdge(i, j, graph[i][j]);
                }
            }
            int ret = sol.Maxflow(0, n - 1);
            if (ret == sumDemand)
                flag = 0;
            else
            {
                aveDemand += ((sumDemand - ret) / edgeAllocationSequence.size()) + 5;
                //aveDemand += ((sumDemand - ret) / edgeAllocationSequence.size()) + 1;
                for (int y : edgeAllocationSequence)
                {
                    edgeWantAlloc[y] = min(aveDemand, edgeLeftBandwidth[k][y]);
                }
            }
        }
        // 分配成功后对数据进行保存
        for (int i = 0; i < sol.edges.size(); i++)
        {
            int from = sol.edges[i].from, to = sol.edges[i].to, val = sol.edges[i].flow;
            if (from == 0 && to >= 1 && to <= N)
            { // 边缘节点分配出去的带宽和
                int y = to - 1;
                edgeLeftBandwidth[k][y] -= val;
                edgeUsedBandwidth[k][y] += val;
            }
            else if (from >= 1 && from <= N && to >= N + 1 && to <= N + M)
            {
                int x = to - N - 1, y = from - 1;
                allot[k][x][y] += val;
            }
            else if (from >= N + 1 && from <= N + M && to == N + M + 1)
            {
                int x = from - N - 1;
                clientLeftDemand[k][x] -= val;
            }
        }
    }
}