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
        if ((sub = strstr(buf, str_qosConstraint)) != NULL) {
            int val = 0;
            for (int i = strlen(str_qosConstraint) + 1; buf[i] != '\r'; i++) {
                val *= 10;
                val += buf[i] - '0';
            }
            qosConstraint = val;
        }
        if ((sub = strstr(buf, str_baseCost)) != NULL) {
            int val = 0;
            for (int i = strlen(str_baseCost) + 1; buf[i] != '\r'; i++) {
                val *= 10;
                val += buf[i] - '0';
            }
            baseCost = val;
        }
        if ((sub = strstr(buf, str_centerCost)) != NULL) {
            double val = 0;
            double dec_mul = 0.1;
            int i = strlen(str_centerCost) + 1;
            for (; buf[i] != '\r'; i++) {
                if (buf[i] == '.') {
                    i++;
                    break;
                }
                val *= 10;
                val += buf[i] - '0';
            }
            for (; buf[i]; i++) {
                val += dec_mul * (buf[i] - '0');
                dec_mul /= 10;
            }
            centerCost = val;
        }
        bzero(buf, sizeof(buf));
    }
    MyPrint("文件已读完\n");
}
void Config::showMsg() {
    std::cout << "+------------------------------------------------------+" << endl;
    std::cout << "|-----------------Show config message------------------|" << endl;
    std::cout << "+------------------------------------------------------+" << endl;
    printf("%16s : %d\n", str_qosConstraint, qosConstraint);
    printf("%16s : %d\n", str_baseCost, baseCost);
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
    P(100), //(client.clientList[0].streamDeand[0].size()),
    delay(M, vector<int>(N, 0)),
    access(M, vector<int>(N, 0)),
    clientLinkEdge(M, vector<int>()),
    edgeLinkClinet(N, vector<int>()),
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
                    if (delay[y][x] >= config.qosConstraint) access[y][x] = 0;
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
        if (delay[y][x] >= config.qosConstraint) access[y][x] = 0;
        else access[y][x] = 1;
        lineIndex++;
        bzero(buf, sizeof(buf));
    }
    for (int y = 0; y < N; y++) {
        for (int x = 0; x < M; x++) {       // 获取相连的边缘节点和客户节点
            if (access[x][y] == 0) continue;
            clientLinkEdge[x].push_back(y);
            edgeLinkClinet[y].push_back(x);
        }
    }
    readFile.close();
    MyPrint("%s文件已读完\n", file);
}
Solution::~Solution() {
    fout.close();
#ifdef TEST
    int index = (int)ceil(T * 0.95) - 1;
    //if (historyAlloc[0].size() != timeLen) return;
    fout.open("../output/data.txt");
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

    fout.open("../output/dataSortEdge.txt");
    unsigned long long grade = 0;
    for (int i = 0; i < N; i++) {
        sort(historyAlloc[i].begin(), historyAlloc[i].end());
        int wj = historyAlloc[i][index].first;
        int Cj = edge.bandwidth[i];
        int flag = 0;
        for (int t = 0; t < T; t++) {
            if (historyAlloc[i][t].first > 0) flag = 1;
        }
    }
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
    cout << "constraint = " << config.qosConstraint << endl;
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
void Solution::init(vector<vector<vector<int>>>& alloc) {
    for (int k = 0; k < T; k++) {
        for (int x = 0; x < M; x++) {
            int n = client.clientList[x].streamDeand[k].size();
            for (int z = 0; z < n; z++) {
                alloc[k][x].push_back(-1);
            }
        }
    }
}
// 输出函数
void Solution::outputSolution(vector<vector<vector<int>>>& alloc, vector<int>& changeEdge) {
    vector<vector<int>> edgeAllocted(T, vector<int>(N, 0));
    for (int k = 0; k < T; k++) {
        if (k > 0) {
            for (int y = 0; y < changeEdge.size(); y++) {
                if (y > 0) fout << ",";
                fout << edge.name[changeEdge[y]];
            }
            fout << endl;
        }
        for (int x = 0; x < M; x++) {
            int n = alloc[k][x].size();
            int streamAlloctedTime = 0;
            int flag1 = 0;
            vector<vector<int>> allocDection(N, vector<int>());
            for (int z = 0; z < n; z++) {
                int y = alloc[k][x][z];
                if (y == -1) continue;
                if (access[x][y] == 0) cout << "分配错误1:" << k << "时刻客户节点" << x << "使用了不相连的边缘节点" << y << endl;
                edgeAllocted[k][y] += client.clientList[x].streamDeand[k][z].first;
                streamAlloctedTime++;
                if (client.clientList[x].streamDeand[k][z].first > 0) allocDection[y].push_back(z);
            }
            //if (streamAlloctedTime != n) cout << "分配错误2:" << k << "时刻客户节点" << x << "有流没分配" << endl;
            //fout << endl;
            fout << client.clientList[x].name << ":";
            for (int y = 0; y < N; y++) {
                if (allocDection[y].size() == 0) continue;
                if (flag1 == 1) fout << ",";
                flag1 = 1;
                fout << "<" << edge.name[y];
                for (int z = 0; z < allocDection[y].size(); z++) {
                    fout << "," << client.clientList[x].streamDeand[k][allocDection[y][z]].second;
                }
                fout << ">";
            }
            fout << endl;
        }
        for (int y = 0; y < N; y++) {
            if (k > 0) edgeAllocted[k][y] += floor(edgeAllocted[k - 1][y] * 0.05);
            //if (edgeAllocted[k][y] > edge.bandwidth[y]) cout << "分配错误3:" << k << "时刻边缘节点" << y << "分配带宽超过上限" << endl;
            historyAlloc[y][k].first = edgeAllocted[k][y];
            historyAlloc[y][k].second = k;
        }
    }
}
void Solution::getAllocSeq(vector<int>& priorAllocEdge, vector<int>& postAllocEdge, int postNum) {
    // 对边缘节点进行排序：优先对连接数进行排序，其次为带宽大小
    priority_queue<edgeAllocSequence> queEdge;
    unsigned long long sumAllocVal = 0;
    for (int y = 0; y < N; y++) {
        int linkNum = 0;
        for (int x = 0; x < M; x++) {
            if (access[x][y] == 0) continue;
            linkNum++;
        }
        if (linkNum > 0) queEdge.push(edgeAllocSequence(y, edge.bandwidth[y], linkNum));
        // if (edgeLinkClinet.size() > 0) queEdge.push(edgeAllocSequence(y, edge.bandwidth[y], edgeLinkClinet.size()));
    }
    // 选择出1%叠加的边缘节点
    for (int i = 0; i < postNum; i++) {
        postAllocEdge.push_back(queEdge.top().edgeId);
        queEdge.pop();
    }
    // 选择5%叠加分配的边缘节点
    while (!queEdge.empty()) {
        priorAllocEdge.push_back(queEdge.top().edgeId);
        queEdge.pop();
    }
}
void Solution::alloc5Percent(int num, vector<int>& allocEdge, vector<vector<vector<int>>>& allocMark, vector<vector<int>>& edgeUsedBandwidth, vector<vector<vector<int>>>& centralBandwidth, double mul) {
    int sumAllocVal = 0;
    for (auto y : allocEdge) 
    {
        // 选择先对哪个时刻进行分配，分配规则：哪个时刻的客户节点流中的总带宽需求最大先进行分配
        priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> queTime;
        vector<int> allocLimit(T, 0);
        for (int k = 0; k < T; k++) {
            int sum = 0;
            //for (auto x : edgeLinkClinet[y]) {
            for (int x = 0; x < M; x++) {
                if (access[x][y] == 0) continue;
                int n = client.clientList[x].streamDeand[k].size(); // k时刻流的数量
                for (int z = 0; z < n; z++) {
                    if (allocMark[k][x][z] != -1) continue;         // 已经分配过的流就不再参与分配
// 尝试
//                    if (client.clientList[x].streamDeand[k][z].first < config.baseCost) continue;
                    sum += client.clientList[x].streamDeand[k][z].first;
                }
            }
            allocLimit[k] = sum;
            queTime.push(pair<int, int>(sum, k));
        }
        for (int i = 0; i < num; i++) 
        {
            int k = -1, maxVal = -1;
            // 获得流量最大的时刻
            for (int t = 0; t < T; t++) {
                if (allocLimit[t] > maxVal) {
                    k = t;
                    maxVal = allocLimit[k];
                }
            }
            // 获取各个流的和
            vector<pair<int, int>> sum(P);
            vector<priority_queue<streamMsg>> streamQue(P);
            for (int z = 0; z < P; z++) 
            {
                priority_queue<int, vector<int>, less<int>> que;
                for (int x = 0; x < M; x++) {
                    if (access[x][y] == 0) continue;
                    // 尝试
                    //                    if (client.clientList[x].streamDeand[k][z].first < config.baseCost) continue;
                    streamQue[z].push(streamMsg(z, x, client.clientList[x].streamDeand[k][z].first, 0));
                    que.push(client.clientList[x].streamDeand[k][z].first);
                }
                while (!que.empty()) {
                    int val = que.top();
                    que.pop();
                    if (sum[z].first + val > edge.bandwidth[y]) continue;
                    sum[z].first += val;
                }
                sum[z].second = z;
            }
            sort(sum.begin(), sum.end());
            for (int i = P - 1; i >= 0; i--) {
                int z = sum[i].second;
                while (!streamQue[z].empty()) {
                    streamMsg temp = streamQue[z].top();
                    streamQue[z].pop();
                    int x = temp.clientId, demand = temp.demand;
                    if (allocMark[k][x][z] != -1) continue;
                    //if (edgeUsedBandwidth[k][y] > ceil(edge.bandwidth[y] * 0.95) - 1) break;
                    if (edgeUsedBandwidth[k][y] != 0 && k == 0 && edgeUsedBandwidth[k][y] + demand > ceil(edge.bandwidth[y] * 0.95) - 1) continue;
                    if (edgeUsedBandwidth[k][y] != 0 && k > 0 && edgeUsedBandwidth[k][y] + demand > ceil(edge.bandwidth[y] * mul) - 1) continue;
                    if (edgeUsedBandwidth[k][y] + demand > edge.bandwidth[y]) continue;
                    edgeUsedBandwidth[k][y] += demand;
                    centralBandwidth[k][y][z] = max(centralBandwidth[k][y][z], demand);
                    sumAllocVal += demand;
                    allocMark[k][x][z] = y;
                }
            }
            for (int t = k; t < T - 1; t++) {
                int cache = floor(edgeUsedBandwidth[t][y] * (1 - mul));
                if (cache == 0) break;
                allocLimit[t + 1] = min(allocLimit[t + 1], edge.bandwidth[y] - cache);
            }
            /*
            if (k > 0 && edgeUsedBandwidth[k][y] >= floor(edge.bandwidth[y] * (1 - mul))) 
                allocLimit[k - 1] = min(allocLimit[k - 1], (edge.bandwidth[y] - edgeUsedBandwidth[k][y]) * 20);
            */
            if (k > 0 && edgeUsedBandwidth[k][y] >= floor(edge.bandwidth[y] * (1 - mul)) && mul == 0.95) 
                allocLimit[k - 1] = min(allocLimit[k - 1], edge.bandwidth[y] - edgeUsedBandwidth[k][y] * 20);
            else if (k > 0 && edgeUsedBandwidth[k][y] >= floor(edge.bandwidth[y] * (1 - mul)) && mul == 0.99) 
                allocLimit[k - 1] = min(allocLimit[k - 1], edge.bandwidth[y] - edgeUsedBandwidth[k][y] * 100);

            allocLimit[k] = 0;
        }
    }
    //cout << "5%内白嫖了带宽和为 : " << sumAllocVal << endl;
}
// 第二次分配，分配思想：尽可能让每个边缘节点分配到baseCost的值
void Solution::allocEdge2BaseCost(vector<int>& allocEdge, vector<vector<vector<int>>>& allocMark, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& cache, vector<vector<vector<int>>>& centralBandwidth) {
    int sumAllocVal = 0;
    for (int k = 0; k < T; k++) {
        // 对流进行排序：大顶堆，对需求最大的流先进行分配，当流的大小一样时，从连接数最小的开始分配
        priority_queue<streamMsg> streamQue;
        for (int x = 0; x < M; x++) {
            int linkNum = 0;
            for (int y = 0; y < N; y++) {
                if (access[x][y] == 0) continue;
                linkNum++;
            }
            for (int z = 0; z < client.clientList[x].streamDeand[k].size(); z++) {
                if (allocMark[k][x][z] != -1) continue; // 表示已经分配过, 不进队列
                if (client.clientList[x].streamDeand[k][z].first > config.baseCost) continue;       // 流的带宽值超过了caseCost
                streamQue.push(streamMsg(z, x, client.clientList[x].streamDeand[k][z].first, linkNum));
            }
        }
        // 对流进行分配,尽可能得怼到baseCost
        while (!streamQue.empty()) {
            streamMsg temp = streamQue.top();
            streamQue.pop();
            int z = temp.streamId, x = temp.clientId, demand = temp.demand;
            if (demand > config.baseCost) continue;
            int objy = -1;
            for (auto y : allocEdge) {
                if (access[x][y] == 0) continue;    // 延迟不满足，不进行分配
                if (edgeUsedBandwidth[k][y] + cache[k][y] + demand > config.baseCost) continue;   // 边缘节点分配后会超过baseCost
                // 判断分配后是否会导致未来的边缘节点超过上限
                double nextUse = demand;
                bool flag = false;
                for (int t = k + 1; t < T; t++) {
                    nextUse *= 0.05;
                    if (floor(nextUse) == 0) break;
                    if (edgeUsedBandwidth[t][y] + (int)floor(nextUse) > edge.bandwidth[y]) {
                        flag = true;
                        break;
                    }
                }
                if (flag) continue;
                objy = y;
                break;
            }
            // 没有边缘节点可进行分配
            if (objy == -1) continue;
            centralBandwidth[k][objy][z] = max(centralBandwidth[k][objy][z], demand);
            edgeUsedBandwidth[k][objy] += demand;
            allocMark[k][x][z] = objy;
            sumAllocVal += demand;
        }
        // 对下一时刻的已用带宽进行更新
        if (k == T - 1) break;      // 已经是最后一个时刻，不再进行更新
        for (int y = 0; y < N; y++) {
            int nextUse = floor(edgeUsedBandwidth[k][y] * 0.05);
            cache[k + 1][y] = nextUse;
        }
    }
    cout << "95%内白嫖了baseCost带宽和为 : " << sumAllocVal << endl;
}
// 最后的一次分配，分配思想：尽可能分数平均得进行分配，当某个边缘节点的带宽已经超过95%时，不再为其继续分配
void Solution::allocLeftDemand(vector<vector<vector<int>>>& allocMark, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& cache, vector<vector<vector<int>>>& centralBandwidth, vector<int>& sourceAllocEdge, vector<int>& changeAllocEdge) {
    // 选择先对哪个边缘节点进行分配, 使该节点的 根号edgeUsedBandwidth / edge.bandwidth尽可能的保持一样，当一样时就选择使用总带宽更大的一个边缘节点
    set<int> sourceEdge;
    set<int> changeEdge;
    for (auto y : sourceAllocEdge) sourceEdge.insert(y);
    for (auto y : changeAllocEdge) changeEdge.insert(y);
    for (int k = 0; k < T; k++) 
    {
        // 对流进行排序：大顶堆，对需求最大的流先进行分s配，当流的大小一样时，从连接数最小的开始分配
        priority_queue<streamMsg> streamQue;
        for (int x = 0; x < M; x++) {
            int linkNum = 0;
            for (int y1 = 0; y1 < N; y1++) {
                if (access[x][y1] == 0) continue;
                linkNum++;
            }
            for (int z = 0; z < client.clientList[x].streamDeand[k].size(); z++) {
                if (allocMark[k][x][z] != -1) continue; // 表示已经分配过, 不进队列
                streamQue.push(streamMsg(z, x, client.clientList[x].streamDeand[k][z].first, linkNum));
            }
        }
        // 对流进行分配：尽可能的平均分
        while (!streamQue.empty()) {
            streamMsg temp = streamQue.top();
            int z = temp.streamId, x = temp.clientId, demand = temp.demand;
            streamQue.pop();
            int objy = -1;
            double minAddVal = INT64_MAX;
            for (int y = 0; y < N; y++) {
                if (access[x][y] == 0) continue;    // 延迟不满足，不进行分配
                if (edgeUsedBandwidth[k][y] + cache[k][y] + demand > edge.bandwidth[y]) continue; // 边缘节点不够分
                double nextUse = demand;
                bool flag = false;
                for (int t = k + 1; t < T; t++) {
                    double mul = 0;
                    if (sourceEdge.find(y) != sourceEdge.end()) mul = 0.95;     // 原来的边缘节点
                    else mul = 0.99;            // 表示选中的20个边缘节点 
                    nextUse *= (1 - mul);
                    if (floor(nextUse) == 0) break;
                    if (edgeUsedBandwidth[t][y] + cache[k][y] + (int)floor(nextUse) > edge.bandwidth[y]) {
                        flag = true;
                        break;
                    }
                }
                if (flag) continue;
                double newVal = config.baseCost;
                if (edgeUsedBandwidth[k][y] + demand > config.baseCost) {
                    newVal = edgeUsedBandwidth[k][y] + demand + (edgeUsedBandwidth[k][y] + demand - config.baseCost) * (edgeUsedBandwidth[k][y] + demand - config.baseCost) / (double)edge.bandwidth[y];
                }
                newVal += max(centralBandwidth[k][y][z], demand) * config.centerCost;
                double addVal = newVal;// - sourceVal;
                if (addVal < minAddVal) {
                    minAddVal = addVal;
                    objy = y;
                }
            }
            // 没有边缘节点可进行分配
            if (objy == -1) cout << "###" << k << "时刻有流无法进行分配, 该流的大小为" << demand << endl;
            centralBandwidth[k][objy][z] = max(centralBandwidth[k][objy][z], demand);
            edgeUsedBandwidth[k][objy] += demand;
            allocMark[k][x][z] = objy;
        }
        // 对下一时刻的已用带宽进行更新
        if (k == T - 1) break;      // 已经是最后一个时刻，不再进行更新
        for (int y = 0; y < N; y++) 
        {
            double mul = 0;
            if (sourceEdge.find(y) != sourceEdge.end()) mul = 0.95;     // 原来的边缘节点
            else mul = 0.99;            // 表示选中的20个边缘节点 
            if (k == 0) mul = 0.95;
            int nextUse = floor(edgeUsedBandwidth[k][y] * (1 - mul));
            cache[k + 1][y] = nextUse;
            edgeUsedBandwidth[k + 1][y] += nextUse;
        }
    }
}
// 一下两个函数在复赛现场均可用:
// work为单词测试
// workFindBestAns为调参寻找最优答案
// 在函数中可以选择加入二次分配的函数(实验证明效果不理想)、也可以对预分配函数进行替换
void Solution::work() {
    // 输出内容为哪个时刻的哪个客户节点的哪个流分配给了哪个边缘节点 <边缘节点, 流>
    vector<vector<vector<int>>> allocMark(T, vector<vector<int>>(M, vector<int>()));
    vector<vector<int>> edgeUsedBandwidth(T, vector<int>(N, 0));
    vector<vector<int>> cache(T, vector<int>(N, 0));
    vector<vector<vector<int>>> centralBandwidth(T, vector<vector<int>>(N, vector<int>(P, 0)));
    vector<int> changeAllocEdge;
    vector<int> sourceAllocEdge;
    int index = (int)ceil(T * 0.95) - 1;
    int num = T - index - 1;       // 每个客户节点有biggerIndexNum次来尽最大能力的进行分配
    int _num = T - index - 1;
    if (num < 0) num = 0;
    cout << "index = " << index << ", num = " << num << endl;
    cout << "M = " << M << ", N = " << N << endl;
    init(allocMark);
    // 分配出谁先分配谁后分配
    getAllocSeq(sourceAllocEdge, changeAllocEdge, 20);
    //cout << endl;
    // priorAllocEdge的分配
    alloc5Percent(num, changeAllocEdge, allocMark, edgeUsedBandwidth, centralBandwidth, 0.99);  // 预分配步骤
    alloc5Percent(num, sourceAllocEdge, allocMark, edgeUsedBandwidth, centralBandwidth, 0.95);  // 预分配步骤
    //allocEdge2BaseCost(priorAllocEdge, allocMark, edgeUsedBandwidth, cache, centralBandwidth);  // 分配至baseCost
    // postAllocEdge的分配
    //alloc5Percent(num, postAllocEdge, allocMark, edgeUsedBandwidth, centralBandwidth);  // 预分配步骤
    //allocEdge2BaseCost(postAllocEdge, allocMark, edgeUsedBandwidth, cache, centralBandwidth);  // 分配至baseCost
    // 分配剩下的
    allocLeftDemand(allocMark, edgeUsedBandwidth, cache, centralBandwidth, sourceAllocEdge, changeAllocEdge);
    double gradeEdge = calculateEdgeGrade(allocMark, sourceAllocEdge, changeAllocEdge);
    double gradeCentre = calculateCentreGrade(allocMark);
    cout << "边缘节点成绩为: " << gradeEdge << ", 中心节点成绩为：" << gradeCentre << ", 本次总成绩为：" << floor(gradeEdge + gradeCentre + 0.5) << endl;
    rellocFunc(allocMark, index, _num, edgeUsedBandwidth, cache, sourceAllocEdge, changeAllocEdge);
    gradeEdge = calculateEdgeGrade(allocMark, sourceAllocEdge, changeAllocEdge);
    gradeCentre = calculateCentreGrade(allocMark);                                                // 微调
    cout << "微调后 - 边缘节点成绩为: " << gradeEdge << ", 中心节点成绩为：" << gradeCentre << ", 本次总成绩为：" << floor(gradeEdge + gradeCentre + 0.5) << endl;
    outputSolution(allocMark, changeAllocEdge);
}
void Solution::workFindBestAns() {
    // 输出内容为哪个时刻的哪个客户节点的哪个流分配给了哪个边缘节点 <边缘节点, 流>
    vector<vector<vector<int>>> ans(T, vector<vector<int>>(M, vector<int>()));
    vector<vector<int>> usedBandwidth(T, vector<int>(N, 0));
    vector<vector<int>> _cache(T, vector<int>(N, 0));
    vector<vector<vector<int>>> _centralBandwidth(T, vector<vector<int>>(N, vector<int>(P, 0)));
    vector<int> changeEdge;
    vector<int> sourceEdge;
    int minGrade = 0X7FFFFFFF;
    struct timeval start, now;
    gettimeofday(&start, NULL);
    gettimeofday(&now, NULL);
    int index = (int)ceil(T * 0.95) - 1;
    int biggerNum = T - index - 1;       // 每个客户节点有biggerIndexNum次来尽最大能力的进行分配
    int _biggerNum = T - index - 1;
    int err = 1;
    if (err == 0) err = 1;
    if (biggerNum < 0) biggerNum = 0;
    cout << "index = " << index << ", num = " << biggerNum << endl;
    cout << "M = " << M << ", N = " << N << endl;
    while (biggerNum >= 0 && now.tv_sec - start.tv_sec < 260) {
        vector<vector<vector<int>>> allocMark(T, vector<vector<int>>(M, vector<int>()));
        vector<vector<int>> edgeUsedBandwidth(T, vector<int>(N, 0));
        vector<vector<int>> cache(T, vector<int>(N, 0));
        vector<vector<vector<int>>> centralBandwidth(T, vector<vector<int>>(N, vector<int>(P, 0)));
        vector<int> changeAllocEdge;
        vector<int> sourceAllocEdge;
        init(allocMark);
        // 分配出谁先分配谁后分配
        getAllocSeq(sourceAllocEdge, changeAllocEdge, 20);
        alloc5Percent(biggerNum, changeAllocEdge, allocMark, edgeUsedBandwidth, centralBandwidth, 0.99);  // 预分配步骤
        alloc5Percent(biggerNum, sourceAllocEdge, allocMark, edgeUsedBandwidth, centralBandwidth, 0.95);  // 预分配步骤
        // priorAllocEdge的分配
        //alloc5Percent(biggerNum, priorAllocEdge, allocMark, edgeUsedBandwidth, centralBandwidth, 0.95);  // 预分配步骤
        //allocEdge2BaseCost(priorAllocEdge, allocMark, edgeUsedBandwidth, cache, centralBandwidth);  // 分配至baseCost
        //// postAllocEdge的分配
        //alloc5Percent(biggerNum, postAllocEdge, allocMark, edgeUsedBandwidth, centralBandwidth);  // 预分配步骤
        //allocEdge2BaseCost(postAllocEdge, allocMark, edgeUsedBandwidth, cache, centralBandwidth);  // 分配至baseCost
        // 分配剩下的
        allocLeftDemand(allocMark, edgeUsedBandwidth, cache, centralBandwidth, sourceAllocEdge, changeAllocEdge);
        //rellocFunc(allocMark, index, _biggerNum, edgeUsedBandwidth, cache);   
        double gradeEdge = calculateEdgeGrade(allocMark, sourceAllocEdge, changeAllocEdge);
        double gradeCentre = calculateCentreGrade(allocMark);
        int nowGrade = floor(gradeEdge + gradeCentre + 0.5);
        if (gradeEdge != -1 && nowGrade < minGrade) {
            _cache = cache;
            minGrade = nowGrade;
            ans = allocMark;
            usedBandwidth = edgeUsedBandwidth;
            changeEdge = changeAllocEdge;
            sourceEdge = sourceAllocEdge;
        }
        cout << biggerNum << " : 边缘节点成绩为: " << gradeEdge << ", 中心节点成绩为：" << gradeCentre << ", 本次总成绩为：" << (gradeEdge == -1 ? -1 : nowGrade) << endl;
        gettimeofday(&now, NULL);
        biggerNum -= err;
    }
    double gradeEdge = calculateEdgeGrade(ans, sourceEdge, changeEdge);
    double gradeCentre = calculateCentreGrade(ans);
    int nowGrade = floor(gradeEdge + gradeCentre + 0.5);
    cout << "边缘节点成绩为: " << gradeEdge << ", 中心节点成绩为：" << gradeCentre << ", 本次总成绩为：" << floor(gradeEdge + gradeCentre + 0.5) << endl;
    rellocFunc(ans, index, _biggerNum, usedBandwidth, _cache, sourceEdge, changeEdge);
    gradeEdge = calculateEdgeGrade(ans, sourceEdge, changeEdge);
    gradeCentre = calculateCentreGrade(ans);
    nowGrade = floor(gradeEdge + gradeCentre + 0.5);
    cout << "微调后 - 边缘节点成绩为: " << gradeEdge << ", 中心节点成绩为：" << gradeCentre << ", 本次总成绩为：" << floor(gradeEdge + gradeCentre + 0.5) << endl;
    outputSolution(ans, changeEdge);
}
void Solution::rellocFunc(vector<vector<vector<int>>>& alloc, int index, int num, vector<vector<int>>& edgeUsedBandwidth, vector<vector<int>>& cache, vector<int>& sourceAllocEdge, vector<int>& changeAllocEdge) {
    // 记录哪个边缘节点在哪个时刻用了什么流
    vector<vector<set<pair<int, int>>>> allocSet(N, vector<set<pair<int, int>>>(T));
    // 记录哪个边缘节点在哪个时刻用了多少流
    vector<vector<pair<int, int>>> nowAlloc(N, vector<pair<int, int>>(T));
    vector<int> val95(N, 0);
    set<int> sourceEdge;
    set<int> changeEdge;
    for (auto y : sourceAllocEdge) sourceEdge.insert(y);
    for (auto y : changeAllocEdge) changeEdge.insert(y);
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
            nowAlloc[y][k] = pair<int, int>(edgeUsedBandwidth[k][y], k);
        }
    }
    for (int y = 0; y < N; y++) {
        // 更新95%点
        for (int y1 = 0; y1 < N; y1++) {
            priority_queue<int, vector<int>, greater<int>> que;
            for (int k = 0; k < T; k++) {
                que.push(nowAlloc[y1][k].first);
                while (que.size() > num + 1) que.pop();
            }
            val95[y1] = que.top();
        }
        vector<pair<int, int>> nowy = nowAlloc[y];
        sort(nowy.begin(), nowy.end());
        for (int t = index; t > -1; t--) {
            if (nowy[t].first <= config.baseCost) break;
            int k = nowy[t].second;
            set<pair<int, int>> st = allocSet[y][k];
            for (auto temp : st) {
                int x = temp.first, z = temp.second;    // 选中流，对流进行分配
                for (int y1 = 0; y1 < N; y1++) {
                    if (y == y1) continue;
                    if (access[x][y1] == 0) continue;
                    int y1UsedBandwidth = nowAlloc[y1][k].first + cache[k][y1];    // 想尝试分配的边缘节点用了多少流
                    int limit = val95[y1];                          // 被分配的边缘节点的95位置的带宽值
                    int demand = client.clientList[x].streamDeand[k][z].first;  // 当前选中流的大小
                    int flag = 0;
                    if (y1UsedBandwidth + demand <= limit && nowAlloc[y1][k].first + floor(demand * 0.05) <= limit) {        // 在95%内
                        flag = 1;
                    }
                    else if (y1UsedBandwidth == limit && y1UsedBandwidth + demand <= config.baseCost) { //在95%上且小于baseCost
                        flag = 2;
                    }
                    //else if (y1UsedBandwidth > limit && y1UsedBandwidth + demand <= edge.bandwidth[y1]) {   // 在5%中，此时判断是否超上限
                    //    flag = 3;  
                    //}
                    int addVal = demand;
                    // 判断对后续的影响
                    if (flag == 1) {
                        double mul = 0;
                        if (sourceEdge.find(y1) != sourceEdge.end()) mul = 0.95;     // 原来的边缘节点
                        else mul = 0.99;            // 表示选中的20个边缘节点 
                        for (int t = k + 1; t < T; t++) {
                            addVal = floor(addVal * (1 - mul));
                            if (addVal == 0) break;
                            if (nowAlloc[y1][t].first + cache[t][y1] + addVal >= edge.bandwidth[y]) {
                                flag = 0;
                                break;
                            }
                        }
                    }
                    if (flag > 0) {
                        //cout << k << "时刻，流" << z << "从" << y << "迁移至" << y1 << "，标记为" << flag << endl; 
                        nowAlloc[y1][k].first += demand;
                        nowAlloc[y][k].first -= demand;
                        alloc[k][x][z] = y1;
                        allocSet[y][k].erase(temp);
                        allocSet[y1][k].insert(temp);
                        // 此处不加不知道是否合理
                                                // 对两个缓存带宽进行更新
                        //addVal = demand;
                        double mul = 0;
                        if (sourceEdge.find(y) != sourceEdge.end()) mul = 0.95;     // 原来的边缘节点
                        else mul = 0.99;            // 表示选中的20个边缘节点 
                        for (int t = k; t < T; t++) 
                        {
                            addVal = floor(addVal * (1 - mul));
                            if (addVal == 0) break;
                            cache[t][y] -= addVal;
                        }
                        //addVal = demand;
                        if (sourceEdge.find(y1) != sourceEdge.end()) mul = 0.95;     // 原来的边缘节点
                        else mul = 0.99;            // 表示选中的20个边缘节点 
                        for (int t = k; t < T; t++) 
                        {
                            addVal = floor(addVal * (1 - mul));
                            if (addVal == 0) break;
                            cache[t][y1] += addVal;
                        }
                        break;
                    }
                }
            }
        }
    }
}
double Solution::calculateEdgeGrade(vector<vector<vector<int>>>& alloc, vector<int>& sourceAllocEdge, vector<int>& changeAllocEdge) {
    set<int> sourceEdge;
    set<int> changeEdge;
    for (auto y : sourceAllocEdge) sourceEdge.insert(y);
    for (auto y : changeAllocEdge) changeEdge.insert(y);
    vector<vector<pair<int, int>>> nowAlloc(N, vector<pair<int, int>>(T));    // 历史分配信息 ： N行timeLen列 <bandwidth, time>
    vector<int> LastEdgeAllocted(N, 0); // 不同边缘节点在上一时刻用掉的带宽
    for (int k = 0; k < T; k++) {   // 计算时序
        vector<int> edgeAllocted(N, 0); // 不同边缘节点在当前时刻用掉的带宽
        for (int x = 0; x < M; x++) {
            int n = alloc[k][x].size();
            int streamAlloctedTime = 0;
            for (int z = 0; z < n; z++) {
                int y = alloc[k][x][z];
                if (y == -1) continue;
                if (access[x][y] == 0) {
                    cout << "error 1" << endl;
                    return -1;
                }
                edgeAllocted[y] += client.clientList[x].streamDeand[k][z].first;
                streamAlloctedTime++;
            }
            if (streamAlloctedTime != n) {
                cout << "error 2" << endl;
                return -1;
            }
        }
        int centreSum = 0;
        for (int y = 0; y < N; y++) {
            double mul = 0;
            if (sourceEdge.find(y) != sourceEdge.end()) mul = 0.95;     // 原来的边缘节点
            else mul = 0.99;            // 表示选中的20个边缘节点 
            edgeAllocted[y] += floor(LastEdgeAllocted[y] * (1 - mul));
            if (edgeAllocted[y] > edge.bandwidth[y]) {
                cout << "error 3" << endl;
                return -1;
            }
            nowAlloc[y][k].first = edgeAllocted[y];
            nowAlloc[y][k].second = k;
        }
        LastEdgeAllocted = edgeAllocted;
    }
    double grade = 0;
    int index = (int)ceil(T * 0.95) - 1;
    for (int i = 0; i < N; i++) {
        sort(nowAlloc[i].begin(), nowAlloc[i].end());
        int wj = nowAlloc[i][index].first;
        int Cj = edge.bandwidth[i];
        int flag = 0;
        double addVal = 0;
        for (int t = 0; t < T; t++) {
            if (nowAlloc[i][t].first > 0) flag = 1;
        }
        if (flag == 0) {
            grade += 0;
        }
        else if (flag == 1 && wj <= config.baseCost) {
            addVal += config.baseCost;
        }
        else {
            addVal += (wj - config.baseCost) * (wj - config.baseCost) / Cj + wj;
        }
        grade += addVal;
    }
    return grade;
}
double Solution::calculateCentreGrade(vector<vector<vector<int>>>& allocMark) {
    vector<vector<vector<int>>> centralBandwidth(T, vector<vector<int>>(N, vector<int>(P, 0)));
    vector<int> centreAllocted(T);
    for (int k = 0; k < T; k++) {   // 计算时序
        for (int y = 0; y < N; y++) {
            for (int z = 0; z < P; z++) {
                centralBandwidth[k][y][z] = 0;
            }
        }
    }
    for (int k = 0; k < T; k++) {
        for (int x = 0; x < M; x++) {
            int n = client.clientList[x].streamDeand[k].size(); // k时刻流的数量
            for (int z = 0; z < P; z++) {
                int y = allocMark[k][x][z];
                centralBandwidth[k][y][z] = max(client.clientList[x].streamDeand[k][z].first, centralBandwidth[k][y][z]);
            }
        }
    }
    for (int k = 0; k < T; k++) {   // 计算时序
        for (int y = 0; y < N; y++) {
            for (int z = 0; z < P; z++) {
                centreAllocted[k] += centralBandwidth[k][y][z];
            }
        }
    }
    sort(centreAllocted.begin(), centreAllocted.end());
    int index = (int)ceil(T * 0.95) - 1;
    return floor(config.centerCost * centreAllocted[index]);
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
// 查看t时刻客户节点x的分配情况
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