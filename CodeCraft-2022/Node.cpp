#include "head.h"
#include "Node.h"
#include <set>
#include <cmath>
#ifdef TEST
const char* configFileName = "../data/config.ini";
const char* siteBandwidthFileName = "../data/site_bandwidth.csv";
const char* demandFileName = "../data/demand.csv";
const char* qosFileName = "../data/qos.csv";
const char* outputFileName = "../output/solution.txt";
#else
const char* configFileName = "/data/config.ini";
const char* siteBandwidthFileName = "/data/site_bandwidth.csv";
const char* demandFileName = "/data/demand.csv";
const char* qosFileName = "/data/qos.csv";
const char* outputFileName = "/output/solution.txt";
#endif
using namespace std;
/*****************************************************************************************
Config类
*****************************************************************************************/
Config::Config() {
    ifstream readFile;
    const char* file;
    file = configFileName;
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open()) {
        MyPrint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[1024];
    char* sub;
    int lineIndex = 1;
    int objIndex = 0;
    bzero(buf, sizeof(buf));
    // 读取文件的第一行，忽略第一行内容
    if (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        lineIndex++;
    }
    else {
        MyPrint("读取%s文件失败\n", file);
        return;
    }
    // 读取文件的2~N行
    bzero(buf, sizeof(buf));
    int confObjIndex = 0;
    while (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        if ((sub = strstr(buf, qosConstraint)) != NULL) {
            int val = 0;
            for (int i = strlen(qosConstraint) + 1; buf[i] != '\r'; i++) {
                val *= 10;
                val += buf[i] - '0';
            }
            constraint = val;
            bzero(buf, sizeof(buf));
        }
        if ((sub = strstr(buf, baseCost)) != NULL) {
            int val = 0;
            for (int i = strlen(baseCost) + 1; buf[i]; i++) {
                val *= 10;
                val += buf[i] - '0';
            }
            cost = val;
            bzero(buf, sizeof(buf));
        }
    }
    MyPrint("文件已读完\n");
}
void Config::showMsg() {
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|-----------------Show config message------------------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    printf("%16s : %d\n", qosConstraint, constraint);
    printf("%16s : %d\n", baseCost, cost);
    //cout << setw(15) << qosConstraint << " : " << constraint << endl;
}
/******************************************************************************************
Edge类
******************************************************************************************/
Edge::Edge() {
    ifstream readFile;
    const char* file;
    file = siteBandwidthFileName;
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open()) {
        MyPrint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[4096];
    int lineIndex = 1;
    int objIndex = 0;
    bzero(buf, sizeof(buf));
    // 读取文件的第一行
    if (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        lineIndex++;
    }
    else {
        MyPrint("读取%s文件失败\n", file);
        return;
    }
    // 读取文件的2~N行
    bzero(buf, sizeof(buf));
    while (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        string substr;
        int t = 0;
        for (int i = 0; buf[i] != '\r'; i++) {
            if (buf[i] == ',') {
                edgeIndex[substr] = objIndex;
                name.push_back(substr);
                substr.clear();
                t++;
            }
            else {
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
    MyPrint("文件已读完\n");
}
void Edge::showMsg() {
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|----------------Show edge node message----------------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    // 输出第一行 ：   name :  bandwidth
    std::cout << std::setw(8) << "index : " << std::setw(8) << "Name : " << std::setw(8) << " Bandwidth" << endl;
    // 输出第2 ~ n行 ： time  name.demand
    for (int i = 0; i < bandwidth.size(); i++) {
        printf("%4d  : %4s  : %8d\n", edgeIndex[name[i]], name[i].c_str(), bandwidth[i]);
        //cout << std::setw(5) << indexEdge[name[i]] << " : " << std::setw(4) << name[i] << "   :" << std::setw(8) << bandwidth[i] << endl;
    }
}
inline int Edge::size() {
    return edgeSize;
}
/******************************************************************************************
Client类
******************************************************************************************/
Client::Client() {
    ifstream readFile;
    const char* file;
    file = demandFileName;
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open()) {
        MyPrint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[4096];
    int lineIndex = 1;
    string substr;
    int index = 0;
    bzero(buf, sizeof(buf));
    // 读取文件的第一行
    if (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        for (int i = 0; buf[i] != '\r'; i++) {
            if (buf[i] == ',') {
                // 除去mtime,stream,后的部分
                if (index > 1) {
                    clientIndex[substr] = index - 2;
                    clientList.push_back(ClientNode(substr));
                }
                substr.clear();
                index++;
            }
            else {
                substr += buf[i];
            }
        }
        clientIndex[substr] = index - 2;
        clientList.push_back(ClientNode(substr));
        lineIndex++;
    }
    else {
        MyPrint("读取%s文件失败\n", file);
        return;
    }
    clientSize = index - 1;
    MyPrint("共有%d个各户节点\n", clientSize);
    // 读取文件的2~N行
    bzero(buf, sizeof(buf));
    string timeName, streamName;
    int timeIndex = -1;
    while (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        substr.clear();
        index = 0;
        for (int i = 0; buf[i] != '\r'; i++) {
            if (buf[i] == ',') {
                if (index == 0) {
                    if (substr != timeName) {   // 获得新的时间序列
                        timeName = substr;
                        timeIndex++;
                        timeList.push_back(substr); // 读取时间信息     
                        for (int j = 0; j < clientSize; j++) {
                            clientList[j].streamDeand.push_back(vector<pair<int, string>>());
                        }
                    }
                }
                else if (index == 1) {
                    streamName = substr;
                }
                else {
                    clientList[index - 2].streamDeand[timeIndex].push_back(pair<int, string>(stoi(substr), streamName)); // 读取带宽需求
                }
                substr.clear();
                index++;
            }
            else {
                substr += buf[i];
            }
        }
        clientList[index - 2].streamDeand[timeIndex].push_back(pair<int, string>{stoi(substr), streamName}); // 读取带宽需求
        lineIndex++;
        bzero(buf, sizeof(buf));
    }
    readFile.close();
    timeLen = timeList.size();
    MyPrint("文件已读完\n");
}
void Client::showMsg() {
    std::cout << "+--------------------------------------------------------+" << endl;
    std::cout << "|----------------Show client node message----------------|" << endl;
    std::cout << "+--------------------------------------------------------+" << endl;
    std::cout << "      Index      :";
    for (int i = 0; i < timeList.size(); i++) {
        cout << timeList[i] << " : " << endl;
        for (auto x : clientList) {
            cout << "    " << x.name << " : ";
            for (auto y : x.streamDeand[i]) {
                cout << '<' << y.second << ": " << y.first << ">, ";
            }
            cout << endl;
        }
    }
}
int Client::size() {
    return clientSize;
}
/******************************************************************************************
Solution
******************************************************************************************/
Solution::Solution() :
    N(edge.size()),
    M(client.size()),
    T(client.timeList.size()),
    delay(M, vector<int>(N, 0)),
    access(M, vector<int>(N, 0)),
    historyAlloc(N, vector<pair<int, int>>(T)) {
    ifstream readFile;
    const char* file;
    file = qosFileName;
    fout.open(outputFileName);
    // 打开路径名为file的文件
    readFile.open(file, ios::in);
    if (!readFile.is_open()) {
        MyPrint("%s文件打开失败\n", file);
        return;
    }
    // 按行读取、解析
    char buf[4096];
    int lineIndex = 1;      // 行号
    int objIndex = 0;
    bzero(buf, sizeof(buf));
    vector<int> disorderForEdge(N, 0);
    vector<int> disorderForClient(M, 0);
    // 读取文件的第一行: 第一行为客户节点，为防止无序，使用了disorderForClient对序号重新标定
    if (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        string substr;
        for (int i = 0; buf[i] != '\r'; i++) {
            if (buf[i] == ',') {
                if (objIndex) {
                    disorderForClient[objIndex - 1] = client.clientIndex[substr];
                }
                substr.clear();
                objIndex++;
            }
            else {
                substr += buf[i];
            }
        }
        disorderForClient[objIndex - 1] = client.clientIndex[substr];
        lineIndex++;
    }
    else {
        MyPrint("读取%s文件失败\n", file);
        return;
    }
    // 读取文件的2~N行: 剩下的为边缘节点，为防止无序，使用了disorderForEdge对序号重新标定
    bzero(buf, sizeof(buf));
    while (readFile.getline(buf, sizeof(buf))) {
        MyPrint("读取%s文件第%d行 : %s\n", file, lineIndex, buf);
        string substr;
        int x, y;
        objIndex = 0;
        for (int i = 0; buf[i] != '\r'; i++) {
            if (buf[i] == ',') {
                if (objIndex) {  //找到的是延迟
                    x = disorderForEdge[lineIndex - 2], y = disorderForClient[objIndex - 1];
                    delay[y][x] = stoi(substr);
                    if (delay[y][x] >= config.constraint) access[y][x] = 0;
                    else access[y][x] = 1;
                }
                else {        //找到的是边缘节点名称
                    disorderForEdge[lineIndex - 2] = edge.edgeIndex[substr];
                }
                objIndex++;
                substr.clear();
            }
            else {
                substr += buf[i];
            }
        }
        x = disorderForEdge[lineIndex - 2], y = disorderForClient[objIndex - 1];
        delay[y][x] = stoi(substr);
        if (delay[y][x] >= config.constraint) access[y][x] = 0;
        else access[y][x] = 1;
        lineIndex++;
        bzero(buf, sizeof(buf));
    }
    readFile.close();
    MyPrint("%s文件已读完\n", file);
}
Solution::~Solution() {
    fout.close();
#ifdef TEST
    int index = (int)ceil(T * 0.95) - 1;
    //if (historyAlloc[0].size() != timeLen) return;
    fout.open("./output/data.txt");
    fout << "       ";
    for (int i = 0; i < N; i++) {
        fout << setw(7) << edge.name[i];
    }
    fout << endl;
    for (int k = 0; k < T; k++) {
        fout << setw(4) << k << " : ";
        for (int i = 0; i < N; i++) {
            fout << setw(7) << historyAlloc[i][k].first;
        }
        fout << endl;
    }
    fout.close();

    unsigned long long grade = 0;
    for (int i = 0; i < N; i++) {
        sort(historyAlloc[i].begin(), historyAlloc[i].end());
        int wj = historyAlloc[i][index].first;
        int Cj = edge.bandwidth[i];
        int flag = 0;
        for (int t = 0; t < T; t++) {
            if (historyAlloc[i][t].first > 0) flag = 1;
        }
        if (flag == 0) {
            grade += 0;
        }
        else if (flag == 1 && wj < config.cost) {
            grade += config.cost;
        }
        else {
            grade += (wj - config.cost) * (wj - config.cost) / Cj + wj;
        }
    }
    //std::cout << "此次得分为 : " << grade << endl;
    fout.open("./output/dataSort.txt");
    fout << "       ";
    for (int i = 0; i < N; i++) {
        fout << setw(13) << edge.name[i];
    }
    fout << endl;
    for (int k = 0; k < T; k++) {

        fout << setw(4) << k << " : ";
        for (int i = 0; i < N; i++) {
            fout << setw(7) << historyAlloc[i][k].first << "(" << setw(4) << historyAlloc[i][k].second << ")";
        }
        fout << endl;
    }
    fout.close();
#endif
}
void Solution::showDelayMatrixMsg() {
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|---------Show Edge2ClientDelay BaseMatrixMsg----------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    cout << "constraint = " << config.constraint << endl;
    std::cout << "edgeName \\ clientName : ";
    for (int i = 0; i < M; i++) {
        printf("%7s  ", client.clientList[i].name.c_str());
    }
    std::cout << endl;
    for (int i = 0; i < N; i++) {
        printf("%-10d -> %7s : ", i + 1, edge.name[i].c_str());
        for (int j = 0; j < M; j++) {
            printf("(%d)%4d  ", access[j][i], delay[j][i]);
            //printf("%d,  ", access[j][i]);
        }
        printf("\n");
    }
}
void Solution::init(vector<vector<vector<int>>>& alloc, vector<vector<int>>& edgeLeftBandwidth) {
    for (int k = 0; k < T; k++) {
        for (int y = 0; y < N; y++) {
            edgeLeftBandwidth[k][y] = edge.bandwidth[y];
        }
        for (int x = 0; x < M; x++) {
            int n = client.clientList[x].streamDeand[k].size();
            for (int z = 0; z < n; z++) {
                alloc[k][x].push_back(-1);
            }
        }
    }
}
void Solution::calculateExpectAlloc(vector<int>& expectAlloc) {
    for (int y = 0; y < N; y++) {
        //if (edge.bandwidth[y] > config.cost) expectAlloc[y] = sqrt(edge.bandwidth[y] * config.cost);
        //else expectAlloc[y] = config.cost;
        expectAlloc[y] = config.cost;
    }
}
// 输出函数
void Solution::outputSolution(vector<vector<vector<int>>>& alloc) {
    int mm = 0;
    for (int y : choosedEdge) {
        if (mm > 0) fout << ",";
        fout << edge.name[y];
        mm = 1;
    }
    fout << endl;
    for (int k = 0; k < T; k++) {
        vector<int> edgeAllocted(N, 0);
        for (int x = 0; x < M; x++) {
            int n = alloc[k][x].size();
            int streamAlloctedTime = 0;
            int flag = 0;
            fout << client.clientList[x].name << ":";
            for (int z = 0; z < n; z++) {
                int y = alloc[k][x][z];
                if (y == -1) continue;
                if (access[x][y] == 0) cout << "分配错误:" << k << "时刻客户节点" << x << "使用了不相连的边缘节点" << y << endl;
                edgeAllocted[y] += client.clientList[x].streamDeand[k][z].first;
                streamAlloctedTime++;
                if (flag == 1) fout << ",";
                flag = 1;
                fout << "<" << edge.name[y] << "," << client.clientList[x].streamDeand[k][z].second << ">";
            }
            if (streamAlloctedTime != n) cout << "分配错误:" << k << "时刻客户节点" << x << "有流没分配" << endl;
            fout << endl;
        }
        for (int y = 0; y < N; y++) {
            if (edgeAllocted[y] > edge.bandwidth[y]) cout << "分配错误:" << k << "时刻边缘节点" << y << "分配带宽超过上限" << endl;
            historyAlloc[y][k].first = edgeAllocted[y];
            historyAlloc[y][k].second = k;
        }
    }
}/*
// 第一轮分配：进行百分之5的分配，希望边缘节点的值尽可能的分配出去
void Solution::alloc5Percent(int biggerNum, vector<vector<vector<int>>> &alloc, vector<vector<int>> &edgeUsedBandwidth, vector<vector<int>> &edgeLeftBandwidth, vector<vector<int>> &edgeAllocIn5) {
    // 选择先对哪个边缘节点进行分配
    priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
    unsigned long long sumAllocVal = 0;
    for (int y = 0; y < N; y++) {
        queEdge.push(pair<int, int>(edge.bandwidth[y], y));
    }
    while (!queEdge.empty()) {
        int y = queEdge.top().second;
        queEdge.pop();
        // 选择先对哪个时刻进行分配，分配规则：哪个时刻的客户节点流中的总带宽需求最大先进行分配
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        for (int k = 0; k < T; k++) {
            int sum = 0;
            for (int x = 0; x < M; x++) {
                if (access[x][y] == 0) continue;
                int n = client.clientList[x].streamDeand[k].size();
                for (int z = 0; z < n; z++) {
                    if (alloc[k][x][z] != -1) continue;         // 已经分配过的流就不再参与分配
                    sum += client.clientList[x].streamDeand[k][z].first;
                }
            }
            queTime.push(pair<int, int>(sum, k));
        }
        for (int i = 0; i < biggerNum; i++) {
            int k = queTime.top().second;
            queTime.pop();
            // 大顶堆，对需求最大的流先进行分配
            priority_queue<streamMsg> streamQue;
            for (int x = 0; x < M; x++) {
                if (access[x][y] == 0) continue;
                int n = client.clientList[x].streamDeand[k].size();
                for (int z = 0; z < n; z++) {
                    if (alloc[k][x][z] != -1) continue;
                    streamQue.push(streamMsg(z, x, client.clientList[x].streamDeand[k][z].first));
                }
            }
            while (!streamQue.empty()) {
                streamMsg temp = streamQue.top();
                streamQue.pop();
                int demand = temp.demand, z = temp.streamId, x = temp.clientId;
                if (demand > edgeLeftBandwidth[k][y]) continue;
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                sumAllocVal += demand;
                alloc[k][x][z] = y;
                edgeAllocIn5[k][y] = 1;
            }
        }
    }
    cout << "5% 共分配的带宽量为 : " << sumAllocVal << endl;
}
// 保险分配法1
void Solution::alloc5Percent(int biggerNum, vector<vector<vector<int>>>& alloc, vector<vector<int>> edgeUsedBandwidth, vector<vector<int>>& edgeLeftBandwidth) {
    // 选择先对哪个边缘节点进行分配
    priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
    for (int y = 0; y < N; y++) {
        queEdge.push(pair<int, int>(edge.bandwidth[y], y));
    }
    while (!queEdge.empty()) {
        int y = queEdge.top().second;
        queEdge.pop();
        // 选择先对哪个时刻进行分配，分配规则：哪个时刻的客户节点流中的总带宽需求最大先进行分配
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        for (int k = 0; k < T; k++) {
            int sum = 0;
            for (int x = 0; x < M; x++) {
                if (access[x][y] == 0) continue;
                int n = client.clientList[x].streamDeand[k].size();
                for (int z = 0; z < n; z++) {
                    if (alloc[k][x][z] != -1) continue;
                    sum += client.clientList[x].streamDeand[k][z].first;
                }
            }
            queTime.push(pair<int, int>(sum, k));
        }
        for (int i = 0; i < biggerNum; i++) {
            int k = queTime.top().second;
            queTime.pop();
            // 选择先对哪个客户节点进行分配，分配规则：能从相连的边缘节点中所获得的带宽和，从小到大进行分配
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> queClient;
            for (int x = 0; x < M; x++) {
                int sum = 0;
                if (access[x][y] == 0) continue;
                for (int y1 = 0; y1 < N; y1++) {
                    if (access[x][y1] == 0) continue;
                    sum += edgeLeftBandwidth[k][y1];
                }
                queClient.push(pair<int, int>(sum, x));
            }
            while (!queClient.empty() && edgeLeftBandwidth[k][y] > 0) {
                int x = queClient.top().second;
                queClient.pop();
                // 选择对哪个客户节点中的哪个流进行分配：对需求量更大的流进行分配
                priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queStream;
                for (int z = 0; z < client.clientList[x].streamDeand[k].size(); z++) {
                    queStream.push(pair<int, int>(client.clientList[x].streamDeand[k][z].first, z));
                }
                while (!queStream.empty() && edgeLeftBandwidth[k][y] > 0) {
                    int z = queStream.top().second;
                    int demand = queStream.top().first;
                    queStream.pop();
                    if (alloc[k][x][z] != -1) continue;                 // 已经分配过了
                    if (demand > edgeLeftBandwidth[k][y]) continue;
                    edgeLeftBandwidth[k][y] -= demand;
                    edgeUsedBandwidth[k][y] += demand;
                    alloc[k][x][z] = y;
                }
            }
        }
    }
}*/

void Solution::alloc5Percent(int biggerNum, int biggerNum2, vector<vector<vector<int>>>& alloc, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeAllocIn5) {
    // 选择先对哪个边缘节点进行分配
    priority_queue<edgeAllocSequence> queEdge;
    unsigned long long sumAllocVal = 0;
    for (int y = 0; y < N; y++) {
        int linkNum = 0;
        for (int x = 0; x < M; x++) {
            if (access[x][y] == 0) continue;
            linkNum++;
        }
        queEdge.push(edgeAllocSequence(y, edge.bandwidth[y], linkNum));
    }
    while (!queEdge.empty()) {
        int y = queEdge.top().edgeId;
        queEdge.pop();
        int Num = biggerNum;
        for (int i = 0; i < 10; i++) {
            if (y == choosedEdge[i]) {
                Num = biggerNum2;
                break;
            }
        }
        // 选择先对哪个时刻进行分配，分配规则：哪个时刻的客户节点流中的总带宽需求最大先进行分配
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        for (int k = 0; k < T; k++) {
            int sum = 0;
            for (int x = 0; x < M; x++) {
                if (access[x][y] == 0) continue;
                int n = client.clientList[x].streamDeand[k].size();
                for (int z = 0; z < n; z++) {
                    if (alloc[k][x][z] != -1) continue;         // 已经分配过的流就不再参与分配
                    sum += client.clientList[x].streamDeand[k][z].first;
                }
            }
            queTime.push(pair<int, int>(sum, k));
        }
        for (int i = 0; i < Num; i++) {
            int k = queTime.top().second;
            queTime.pop();
            // 大顶堆，对需求最大的流先进行分配
            priority_queue<streamMsg> streamQue;
            for (int x = 0; x < M; x++) {
                if (access[x][y] == 0) continue;
                int n = client.clientList[x].streamDeand[k].size();
                int linkNum = 0;
                for (int z = 0; z < n; z++) {
                    if (alloc[k][x][z] != -1) continue;
                    streamQue.push(streamMsg(z, x, client.clientList[x].streamDeand[k][z].first, linkNum));
                }
            }
            while (!streamQue.empty()) {
                streamMsg temp = streamQue.top();
                streamQue.pop();
                int demand = temp.demand, z = temp.streamId, x = temp.clientId;
                if (demand > edgeLeftBandwidth[k][y]) continue;
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                sumAllocVal += demand;
                alloc[k][x][z] = y;
                edgeAllocIn5[k][y] = 1;
            }
        }
    }
}

// 第二轮分配：使边缘节点分配到期望的带宽值
void Solution::allocEdge2ExpectAlloc(vector<vector<vector<int>>>& alloc, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& edgeLeftBandwidth, vector<int>& expectAlloc) {
    for (int k = 0; k < T; k++) {
        // 选择先对哪个边缘节点进行分配, 使该节点尽可能的分配到expectAlloc
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queEdge;
        for (int y = 0; y < N; y++) {
            if (0 == edgeUsedBandwidth[k][y])
                queEdge.push(pair<int, int>(edgeLeftBandwidth[k][y], y));
        }
        while (!queEdge.empty()) {
            int y = queEdge.top().second;
            queEdge.pop();
            // 大顶堆，对需求最大的流先进行分配
            priority_queue<streamMsg> streamQue;
            for (int x = 0; x < M; x++) {
                if (access[x][y] == 0) continue;
                int linkNum = 0;
                for (int y1 = 0; y1 < N; y1++) {
                    if (access[x][y1] == 0) continue;
                    linkNum++;
                }
                for (int z = 0; z < client.clientList[x].streamDeand[k].size(); z++) {
                    if (alloc[k][x][z] != -1) continue; // 表示已经分配过, 不进队列
                    streamQue.push(streamMsg(z, x, client.clientList[x].streamDeand[k][z].first, linkNum));
                }
            }
            while (!streamQue.empty() && edgeUsedBandwidth[k][y] < expectAlloc[y]) {
                streamMsg temp = streamQue.top();
                streamQue.pop();
                int x = temp.clientId, z = temp.streamId, demand = temp.demand;
                if (alloc[k][x][z] != -1) continue;             // 已经分配过了
                if (demand + edgeUsedBandwidth[k][y] > expectAlloc[y]) continue;    // 再进行分配就超过期望值了
                edgeLeftBandwidth[k][y] -= demand;
                edgeUsedBandwidth[k][y] += demand;
                alloc[k][x][z] = y;
            }
        }
    }
}
// 第三轮分配：尽可能的平均分配
void Solution::allocLeftDemand(vector<vector<vector<int>>>& alloc, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& edgeLeftBandwidth, vector<vector<int>>& edgeAllocIn5) {
    // 选择先对哪个边缘节点进行分配, 使该节点的 根号edgeUsedBandwidth / edge.bandwidth尽可能的保持一样，当一样时就选择使用总带宽更大的一个边缘节点
    vector<double> edgeUsedSqrtPersent(N);
    for (int k = 0; k < T; k++) {
        for (int y = 0; y < N; y++) {
            edgeUsedSqrtPersent[y] = sqrt(edgeUsedBandwidth[k][y] / (double)edge.bandwidth[y]);
        }
        // 大顶堆，对需求最大的流先进行分配
        priority_queue<streamMsg> streamQue;
        for (int x = 0; x < M; x++) {
            int linkNum = 0;
            //for (int y1 = 0; y1 < N; y1++) {
            //    if (access[x][y1] == 0) continue;
            //    linkNum++;
            //}
            for (int z = 0; z < client.clientList[x].streamDeand[k].size(); z++) {
                if (alloc[k][x][z] != -1) continue; // 表示已经分配过, 不进队列
                streamQue.push(streamMsg(z, x, client.clientList[x].streamDeand[k][z].first, linkNum));
            }
        }
        while (!streamQue.empty()) {
            streamMsg temp = streamQue.top();
            int z = temp.streamId, x = temp.clientId, demand = temp.demand;
            streamQue.pop();
            int objy = -1;
            double minVal = 1;
            for (int y = 0; y < N; y++) {
                if (access[x][y] == 0) continue;    // 延迟不满足，不进行分配
                if (edgeLeftBandwidth[k][y] < demand) continue; // 边缘节点不够分
                if (edgeAllocIn5[k][y] == 1) {
                    objy = y;
                    break;
                }
                double val = sqrt((edgeUsedBandwidth[k][y] + demand) / (double)edge.bandwidth[y]);
                if (val < minVal) {
                    minVal = val;
                    objy = y;
                }
            }
            edgeLeftBandwidth[k][objy] -= demand;
            edgeUsedBandwidth[k][objy] += demand;
            alloc[k][x][z] = objy;
            edgeUsedSqrtPersent[objy] = (edgeUsedBandwidth[k][objy] - config.cost) / (double)edge.bandwidth[objy] * (edgeUsedBandwidth[k][objy] - config.cost);
            //edgeUsedSqrtPersent[objy] = sqrt(edgeUsedBandwidth[k][objy] / (double)edge.bandwidth[objy]);
        }
    }
}
// 一下两个函数在复赛现场均可用:
// work为单词测试
// workFindBestAns为调参寻找最优答案
// 在函数中可以选择加入二次分配的函数(实验证明效果不理想)、也可以对预分配函数进行替换
void Solution::work() {
    // 输出内容为哪个时刻的哪个客户节点的哪个流分配给了哪个边缘节点 <边缘节点, 流>
    vector<vector<vector<int>>> alloc(T, vector<vector<int>>(M, vector<int>()));
    vector<vector<int>> edgeUsedBandwidth(T, vector<int>(N, 0));
    vector<vector<int>> edgeLeftBandwidth(T, vector<int>(N, 0));
    vector<vector<int>> edgeAllocIn5(T, vector<int>(N, 0));
    vector<int> expectAlloc(N, 0);
    int index1 = (int)ceil(T * 0.95) - 1;
    int index2 = (int)ceil(T * 0.90) - 1;
    int biggerNum = T - index1 - 21;       // 每个客户节点有biggerIndexNum次来尽最大能力的进行分配
    int biggerNum10 = T - index2 - 21;
    int _biggerNum = T - index1 - 1;       // 每个客户节点有biggerIndexNum次来尽最大能力的进行分配
    int _biggerNum10 = T - index2 - 1;
    // 选取10个边缘节点,取流最大的10个
    priority_queue<edgeAllocSequence2> edgeQue10;
    for (int y = 0; y < N; y++) {
        int linkNum = 0;
        for (int x = 0; x < M; x++) {
            if (access[x][y] == 0) continue;
            linkNum++;
        }
        edgeQue10.push(edgeAllocSequence2(y, edge.bandwidth[y], linkNum));
    }
    for (int i = 0; i < 10; i++) {
        edgeAllocSequence2 temp = edgeQue10.top();
        edgeQue10.pop();
        choosedEdge.push_back(temp.edgeId);
    }
    if (biggerNum < 0) biggerNum = 0;
    if (biggerNum10 < 0) biggerNum10 = 0;
    cout << "T = " << T << endl;
    cout << "index1 = " << index1 << ", index2 = " << index2 << ", num1 = " << biggerNum << ", num2 = " << biggerNum10 << endl;
    cout << "M = " << M << ", N = " << N << endl;
    init(alloc, edgeLeftBandwidth);
    alloc5Percent(biggerNum, biggerNum10, alloc, edgeUsedBandwidth, edgeLeftBandwidth, edgeAllocIn5);
    cout << "预分配1完成" << endl;
    allocLeftDemand(alloc, edgeUsedBandwidth, edgeLeftBandwidth, edgeAllocIn5);
    cout << "分配完成" << endl;
    int grade = calculateGrade(alloc);
    cout << "优化前成绩" << grade << endl;
    rellocFunc(alloc, index1, index2, _biggerNum, _biggerNum10);
    grade = calculateGrade(alloc);
    cout << "优化后成绩" << grade << endl;
    outputSolution(alloc);
}
void Solution::workFindBestAns() {
    // 输出内容为哪个时刻的哪个客户节点的哪个流分配给了哪个边缘节点 <边缘节点, 流>
    vector<vector<vector<int>>> ans(T, vector<vector<int>>(M, vector<int>()));
    int minGrade = 0X7FFFFFFF;
    struct timeval start, now;
    gettimeofday(&start, NULL);
    gettimeofday(&now, NULL);
    int index1 = (int)ceil(T * 0.95) - 1;
    int index2 = (int)ceil(T * 0.90) - 1;
    int biggerNum = T - index1 - 15;       // 每个客户节点有biggerIndexNum次来尽最大能力的进行分配
    int _biggerNum = T - index1 - 1;
    int biggerNum10 = T - index2 -15;
    int _biggerNum10 = T - index2 - 1;
    // 选取10个边缘节点,取流最大的10个
    priority_queue<edgeAllocSequence2> edgeQue10;
    for (int y = 0; y < N; y++) {
        int linkNum = 0;
        for (int x = 0; x < M; x++) {
            if (access[x][y] == 0) continue;
            linkNum++;
        }
        edgeQue10.push(edgeAllocSequence2(y, edge.bandwidth[y], linkNum));
    }
    for (int i = 0; i < 10; i++) {
        edgeAllocSequence2 temp = edgeQue10.top();
        edgeQue10.pop();
        choosedEdge.push_back(temp.edgeId);
    }
    int err = 1;
    if (err == 0) err = 1;
    if (biggerNum < 0) biggerNum = 0;

    cout << "index1 = " << index1 << ", index2 = " << index2 << ", num1 = " << biggerNum << ", num2 = " << biggerNum10 << endl;
    cout << "M = " << M << ", N = " << N << endl;
    while (biggerNum >= 0 && now.tv_sec - start.tv_sec < 240) 
    {
        vector<vector<vector<int>>> alloc(T, vector<vector<int>>(M, vector<int>()));
        vector<vector<int>> edgeUsedBandwidth(T, vector<int>(N, 0));
        vector<vector<int>> edgeLeftBandwidth(T, vector<int>(N, 0));
        vector<vector<int>> edgeAllocIn5(T, vector<int>(N, 0));
        vector<int> expectAlloc(N, 0);
        init(alloc, edgeLeftBandwidth);
        alloc5Percent(biggerNum, biggerNum10, alloc, edgeUsedBandwidth, edgeLeftBandwidth, edgeAllocIn5);
        allocLeftDemand(alloc, edgeUsedBandwidth, edgeLeftBandwidth, edgeAllocIn5);
        rellocFunc(alloc, index1, index2, _biggerNum, _biggerNum10);
        //rellocFunc5(alloc, index1, _biggerNum);
        //rellocFunc10(alloc, index2, _biggerNum10);
        int nowGrade = calculateGrade(alloc);
        if (nowGrade < minGrade) {
            minGrade = nowGrade;
            ans = alloc;
        }
        cout << biggerNum << "成绩为:" << nowGrade << endl;
        gettimeofday(&now, NULL);
        biggerNum -= err;
        biggerNum10 -= err;
    }
    //rellocFunc5(ans, index1, _biggerNum);
    //rellocFunc10(ans, index2, _biggerNum10);

    //rellocFunc(ans, index1, index2, _biggerNum, _biggerNum10);
    int nowGrade = calculateGrade(ans);
    cout << "最后优化成绩为:" << nowGrade << endl;
    outputSolution(ans);
}

void Solution::rellocFunc10(vector<vector<vector<int>>> &alloc, int index90, int biggerNum90)
{
    // 记录哪个边缘节点在哪个时刻用了什么流
    vector<vector<set<pair<int, int>>>> allocSet(N, vector<set<pair<int, int>>>(T));
    // 记录哪个边缘节点在哪个时刻用了多少流
    vector<vector<pair<int, int>>> nowAlloc(N, vector<pair<int, int>>(T));
    vector<int> val90(N, 0);
    for (int k = 0; k < T; k++)
    {
        for (int x = 0; x < M; x++)
        {
            int n = client.clientList[x].streamDeand[k].size();
            for (int z = 0; z < n; z++)
            {
                int y = alloc[k][x][z];
                allocSet[y][k].insert(pair<int, int>(x, z));
            }
        }
    }
    for (int y = 0; y < N; y++)
    {
        for (int k = 0; k < T; k++)
        {
            int sum = 0;
            for (int x = 0; x < M; x++)
            {
                int n = client.clientList[x].streamDeand[k].size();
                for (int z = 0; z < n; z++)
                {
                    if (y == alloc[k][x][z])
                        sum += client.clientList[x].streamDeand[k][z].first;
                }
            }
            nowAlloc[y][k] = pair<int, int>(sum, k);
        }
    }
    for (int y : choosedEdge)//迁移10个90%边缘
    {
        // 更新90%点
        for (int y1 : choosedEdge)
        {
            priority_queue<int, vector<int>, greater<int>> que;
            for (int k = 0; k < T; k++)
            {
                que.push(nowAlloc[y1][k].first);
                while (que.size() > biggerNum90 + 1)
                    que.pop();
            }
            val90[y1] = que.top();
        }
        vector<pair<int, int>> nowy = nowAlloc[y];
        sort(nowy.begin(), nowy.end());
        for (int t = index90; t > -1; t--)
        {
            if (nowy[t].first <= config.cost) break;
            int k = nowy[t].second;
            set<pair<int, int>> st = allocSet[y][k];
            for (auto temp : st)
            {
                int x = temp.first, z = temp.second; // 选中流，对流进行分配
                for (int y1 : choosedEdge)
                {
                    if (y == y1)
                        continue;
                    if (access[x][y1] == 0)
                        continue;
                    int y1UsedBandwidth = nowAlloc[y1][k].first;               // 想尝试分配的边缘节点用了多少流
                    int limit = val90[y1];                                     // 被分配的边缘节点的95位置的带宽值
                    if(limit == 0)continue;
                    if(limit == 0)
                    {
                        cout << "90带宽错误"<<endl;
                    }
                    int demand = client.clientList[x].streamDeand[k][z].first; // 当前选中流的大小
                    if (y1UsedBandwidth < limit && y1UsedBandwidth + demand <= limit)
                    { // 在90%内
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                    else if (y1UsedBandwidth == limit && y1UsedBandwidth + demand <= config.cost)
                    {
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                }
            }
        }
    }
}

void Solution::rellocFunc5(vector<vector<vector<int>>> &alloc, int index, int biggerNum)
{
    // 记录哪个边缘节点在哪个时刻用了什么流
    vector<vector<set<pair<int, int>>>> allocSet(N, vector<set<pair<int, int>>>(T));
    // 记录哪个边缘节点在哪个时刻用了多少流
    vector<vector<pair<int, int>>> nowAlloc(N, vector<pair<int, int>>(T));
    vector<int> val95(N, 0);
    for (int k = 0; k < T; k++)
    {
        for (int x = 0; x < M; x++)
        {
            int n = client.clientList[x].streamDeand[k].size();
            for (int z = 0; z < n; z++)
            {
                int y = alloc[k][x][z];
                allocSet[y][k].insert(pair<int, int>(x, z));
            }
        }
    }
    for (int y = 0; y < N; y++)
    {
        for (int k = 0; k < T; k++)
        {
            int sum = 0;
            for (int x = 0; x < M; x++)
            {
                int n = client.clientList[x].streamDeand[k].size();
                for (int z = 0; z < n; z++)
                {
                    if (y == alloc[k][x][z])
                        sum += client.clientList[x].streamDeand[k][z].first;
                }
            }
            nowAlloc[y][k] = pair<int, int>(sum, k);
        }
    }
    for (int y = 0; y < N; y++)
    {
        //除去90成本的边缘
        if(find(choosedEdge.begin(), choosedEdge.end(), y) != choosedEdge.end()) continue;
        // 更新95%点
        for (int y1 = 0; y1 < N; y1++)
        {
            //除去90成本的边缘
            if(find(choosedEdge.begin(), choosedEdge.end(), y1) != choosedEdge.end()) continue;
            priority_queue<int, vector<int>, greater<int>> que;
            for (int k = 0; k < T; k++)
            {
                que.push(nowAlloc[y1][k].first);
                while (que.size() > biggerNum + 1)
                    que.pop();
            }
            val95[y1] = que.top();
        }
        vector<pair<int, int>> nowy = nowAlloc[y];
        sort(nowy.begin(), nowy.end());
        for (int t = index; t > -1; t--)
        {
            if (nowy[t].first <= config.cost)
                break;
            int k = nowy[t].second;
            set<pair<int, int>> st = allocSet[y][k];
            for (auto temp : st)
            {
                int x = temp.first, z = temp.second; // 选中流，对流进行分配
                for (int y1 = 0; y1 < N; y1++)
                {
                    //除去90成本的边缘
                    if(find(choosedEdge.begin(), choosedEdge.end(), y1) != choosedEdge.end()) continue;
                    if (y == y1) continue;
                    if (access[x][y1] == 0)   continue;
                    int y1UsedBandwidth = nowAlloc[y1][k].first;               // 想尝试分配的边缘节点用了多少流
                    int limit = val95[y1];
                    if(limit == 0)continue;
                    if(limit == 0)
                    {
                        cout << "95带宽错误"<<endl;
                    }
                    int demand = client.clientList[x].streamDeand[k][z].first; // 当前选中流的大小
                    if (y1UsedBandwidth < limit && y1UsedBandwidth + demand <= limit)
                    { // 在95%内
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                    else if (y1UsedBandwidth == limit && y1UsedBandwidth + demand <= config.cost)
                    {
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                    else if (y1UsedBandwidth > limit && y1UsedBandwidth + demand <= edge.bandwidth[y1])
                    {
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                }
            }
        }
    }
}



void Solution::rellocFunc(vector<vector<vector<int>>>& alloc, int index1, int index2, int biggerNum1, int biggerNum2) {
    // 记录哪个边缘节点在哪个时刻用了什么流
    vector<vector<set<pair<int, int>>>> allocSet(N, vector<set<pair<int, int>>>(T));
    // 记录哪个边缘节点在哪个时刻用了多少流
    vector<vector<pair<int, int>>> nowAlloc(N, vector<pair<int, int>>(T));
    vector<int> val95(N, 0);
    for (int k = 0; k < T; k++) {
        for (int x = 0; x < M; x++) {
            int n = client.clientList[x].streamDeand[k].size();
            for (int z = 0; z < n; z++) {
                int y = alloc[k][x][z];
                allocSet[y][k].insert(pair<int, int>(x, z));
            }
        }
    }
    for (int y = 0; y < N; y++) {
        for (int k = 0; k < T; k++) {
            int sum = 0;
            for (int x = 0; x < M; x++) {
                int n = client.clientList[x].streamDeand[k].size();
                for (int z = 0; z < n; z++) {
                    if (y == alloc[k][x][z]) sum += client.clientList[x].streamDeand[k][z].first;
                }
            }
            nowAlloc[y][k] = pair<int, int>(sum, k);
        }
    }
    for (int y = 0; y < N; y++) {
        // 更新95%点或90的点
        int index = index1;     // 从哪里开始
        for (int i = 0; i < 10; i++) {
            if (choosedEdge[i] == y) {          // 10个点中就不再进行重分配
                index = index2;
                break;
            }
        }
        for (int y1 = 0; y1 < N; y1++) {
            int Num = biggerNum1;
            for (int i = 0; i < 10; i++) {
                if (choosedEdge[i] == y1) {// 找10个点
                    Num = biggerNum2;
                    break;
                }
            }
            priority_queue<int, vector<int>, greater<int>> que;
            for (int k = 0; k < T; k++) {
                que.push(nowAlloc[y1][k].first);
                while (que.size() > Num + 1) que.pop();
            }
            val95[y1] = que.top();  // 每个点的评分位置
        }
        vector<pair<int, int>> nowy = nowAlloc[y];
        sort(nowy.begin(), nowy.end());
        for (int t = index; t > -1; t--) {
            if (nowy[t].first <= config.cost) break;
            int k = nowy[t].second;
            set<pair<int, int>> st = allocSet[y][k];
            for (auto temp : st) {
                int x = temp.first, z = temp.second;    // 选中流，对流进行分配
                for (int y1 = 0; y1 < N; y1++) {
                    if (y == y1) continue;
                    if (access[x][y1] == 0) continue;
                    int y1UsedBandwidth = nowAlloc[y1][k].first;    // 想尝试分配的边缘节点用了多少流
                    int limit = val95[y1];                          // 被分配的边缘节点的95位置的带宽值
                    int demand = client.clientList[x].streamDeand[k][z].first;  // 当前选中流的大小
                    if (y1UsedBandwidth < limit && y1UsedBandwidth + demand <= limit) {        // 在95%内
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                    else if (y1UsedBandwidth == limit && y1UsedBandwidth + demand <= config.cost) {
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                    else if (y1UsedBandwidth > limit && y1UsedBandwidth + demand <= edge.bandwidth[y1]) {
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        break;
                    }
                }
            }
        }
    }
}
// 查看t时刻边缘节点y的分配情况
void Solution::showEdgeAllocSituation(vector<vector<vector<int>>>& alloc, int y, int t) {
    cout << endl;
    cout << "边缘节点" << edge.name[y] << "的分配情况如下：" << endl;
    int sum = 0;
    for (int x = 0; x < M; x++) {
        if (access[x][y] == 0) continue;
        int n = client.clientList[x].streamDeand[t].size();
        for (int z = 0; z < n; z++) {
            if (alloc[t][x][z] == y) {
                sum += client.clientList[x].streamDeand[t][z].first;
                cout << client.clientList[x].name << "的" << client.clientList[x].streamDeand[t][z].second << "分配给了" << edge.name[y] << "，分配的值为" << client.clientList[x].streamDeand[t][z].first << endl;
            }
        }
    }
    cout << edge.name[y] << "共被分配了" << sum << endl;
}
// 查看t时刻边缘节点y的分配情况
void Solution::showClientAllocSituation(vector<vector<vector<int>>>& alloc, int x, int t) {
    int sum = 0;
    int n = client.clientList[x].streamDeand[t].size();
    cout << endl;
    cout << client.clientList[x].name << "的分配情况如下" << endl;
    for (int z = 0; z < n; z++) {
        if (alloc[t][x][z] == -1) cout << client.clientList[x].streamDeand[t][z].second << "无带宽需求" << endl;
        sum += client.clientList[x].streamDeand[t][z].first;
        cout << client.clientList[x].streamDeand[t][z].second << "分配给了" << edge.name[alloc[t][x][z]] << "，分配的值为" << client.clientList[x].streamDeand[t][z].first << endl;
    }
    cout << client.clientList[x].name << "共被被分配了" << sum << endl;
}
unsigned long long Solution::calculateGrade(vector<vector<vector<int>>>& alloc) {
    vector<vector<pair<int, int>>> nowAlloc(N, vector<pair<int, int>>(T));    // 历史分配信息 ： N行timeLen列 <bandwidth, time>
    for (int k = 0; k < T; k++) {
        vector<int> edgeAllocted(N, 0);
        for (int x = 0; x < M; x++) {
            int n = alloc[k][x].size();
            int streamAlloctedTime = 0;
            for (int z = 0; z < n; z++) {
                int y = alloc[k][x][z];
                if (y == -1) continue;
                if (access[x][y] == 0) return 0X7fffffff;
                edgeAllocted[y] += client.clientList[x].streamDeand[k][z].first;
                streamAlloctedTime++;
            }
            if (streamAlloctedTime != n) return 0X7fffffff;
        }
        for (int y = 0; y < N; y++) {
            if (edgeAllocted[y] > edge.bandwidth[y]) return 0X7fffffff;
            nowAlloc[y][k].first = edgeAllocted[y];
            nowAlloc[y][k].second = k;
        }
    }
    unsigned long long grade = 0;
    int index1 = (int)ceil(T * 0.95) - 1;
    int index2 = (int)ceil(T * 0.90) - 1;
    for (int i = 0; i < N; i++) {
        sort(nowAlloc[i].begin(), nowAlloc[i].end());
        int index = index1;
        for (auto yyy : choosedEdge) {
            if (yyy == i) index = index2;
        }
        int wj = nowAlloc[i][index].first;
        int Cj = edge.bandwidth[i];
        int flag = 0;
        for (int t = 0; t < T; t++) {
            if (nowAlloc[i][t].first > 0) flag = 1;
        }
        if (flag == 0) {
            grade += 0;
        }
        else if (flag == 1 && wj < config.cost) {
            grade += config.cost;
        }
        else {
            grade += (wj - config.cost) * (wj - config.cost) / Cj + wj;
        }
    }
    return grade;
}