#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <ctime>
#include <functional>
#include <vector>

#define DATABASE_NAME "DBPEDIA"
#define RDF3X_LOAD "/home/remixman/Downloads/gh-rdf3x/bin/rdf3xload"
#define RDF3X_UPDATE "/home/remixman/Downloads/gh-rdf3x/bin/rdf3xupdate"
#define RDF3X_QUERY "/home/remixman/Downloads/gh-rdf3x/bin/rdf3xquery"

void loadData(const char *dbname, const char *path);
size_t getFileSize(const char *fname);
void generateRule5Triple(std::string &result);
void generateRule11Triple(std::string &result);
void generateRule2Triple(std::string &result);
void generateRule3Triple(std::string &result);
void generateRule7Triple(std::string &result);
void generateRule9Triple(std::string &result);
void updateTripleWithRule(const char *ruleFile, std::function<void(std::string &)> generateTripleFunction);

int main()
{
    const clock_t beginLoad = clock();
    loadData(DATABASE_NAME, "./data/dbpedia_2016-10.nt");
    // loadData(DATABASE_NAME, "./data/peel.nt");
    const clock_t endLoad = clock();
    std::cout << "START TIME DATA SIZE : " << getFileSize(DATABASE_NAME) << "\n";

    const clock_t beginAll = clock();
    size_t fileSize = getFileSize(DATABASE_NAME);
    size_t origFileSize = fileSize;
    while (true) {
        updateTripleWithRule("rule5.sparql", generateRule5Triple);
        updateTripleWithRule("rule7.sparql", generateRule7Triple);
        fileSize = getFileSize(DATABASE_NAME);

        if (fileSize <= origFileSize) break;
        origFileSize = fileSize;
    }

    updateTripleWithRule("rule2.sparql", generateRule2Triple);
    updateTripleWithRule("rule3.sparql", generateRule3Triple);
    updateTripleWithRule("rule9sparql", generateRule9Triple);
    updateTripleWithRule("rule11.sparql", generateRule11Triple);

    const clock_t endAll = clock();

    std::cout << "FINISH TIME DATA SIZE : " << getFileSize(DATABASE_NAME) << "\n";

    std::cout << "======================================================================\n";
    std::cout << "LOADING DATA TIME   : " << double(endLoad-beginLoad)/CLOCKS_PER_SEC << "\n";
    std::cout << "OVERALL ENTAIL TIME : " << double(endAll-beginAll)/CLOCKS_PER_SEC << "\n";
    std::cout << "======================================================================\n";

    return 0; 
} 

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

void loadData(const char *dbname, const char *path) {
    char cmdBuf[100] = RDF3X_LOAD;
    strcat(cmdBuf, " ");  strcat(cmdBuf, dbname);
    strcat(cmdBuf, " ");  strcat(cmdBuf, path);
    exec(cmdBuf);
}

size_t getFileSize(const char *fname) {
    char cmdBuf[100] = "stat -c \"%s\" ";
    strcat(cmdBuf, fname);
    return atoi(exec(cmdBuf).c_str());
}

void updateTriple(std::vector<std::string> &triples, const char *dbname) {
    const char *tmp = "tmp-triple.nt";
    std::ofstream outfile(tmp);
    for (auto it = triples.begin(); it != triples.end(); ++it)
        outfile << *it << "\n";
    outfile.close();

    char cmdBuf[100] = RDF3X_UPDATE;
    strcat(cmdBuf, " ");  strcat(cmdBuf, dbname);
    strcat(cmdBuf, " ");  strcat(cmdBuf, tmp);
    exec(cmdBuf);
}

void updateTripleWithRule(const char *ruleFile, std::function<void(std::string &)> generateTripleFunction) {
    char queryCmd[100] = RDF3X_QUERY;
    strcat(queryCmd, " ");  strcat(queryCmd, DATABASE_NAME);
    strcat(queryCmd, " ");  strcat(queryCmd, ruleFile);
    std::string output = exec(queryCmd);

    if (output == "<empty result>\n") {
        std::cout << "GENERATED TRIPLE FROM " << ruleFile << " : 0\n";
        return;
    }

    std::vector<std::string> triples;
    std::string delim = "\n";

    auto start = 0U;
    auto end = output.find(delim);
    while (end != std::string::npos)
    {
        std::string result = output.substr(start, end - start); //std::cout << result << "\n";
        generateTripleFunction(result); //std::cout << result << "\n";
        
        triples.push_back(result);
        start = end + delim.length();
        end = output.find(delim, start);
    }

    std::cout << "GENERATED TRIPLE FROM " << ruleFile << " : " << triples.size() << "\n";
    updateTriple(triples, DATABASE_NAME);
}

void generateRule5Triple(std::string &result) {
    auto spacePos = result.find(" ");
    result.insert(spacePos + 1, "<http://www.w3.org/2000/01/rdf-schema#subPropertyOf> ");
    result += " .";
}

void generateRule11Triple(std::string &result) {
    // return '  "' + r[0] + '" rdfs:subClassOf "' + r[1] + '" . \n'
    auto spacePos = result.find(" ");
    result.insert(spacePos + 1, "<http://www.w3.org/2000/01/rdf-schema#subClassOf> ");
    result += " .";
}

void generateRule2Triple(std::string &result) {
    auto spacePos = result.find(" ");
    result.insert(spacePos + 1, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ");
    result += " .";
}

void generateRule3Triple(std::string &result) {
    auto spacePos = result.find(" ");
    result.insert(spacePos + 1, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ");
    result += " .";
}

void generateRule7Triple(std::string &result) {
    result += " .";
}

void generateRule9Triple(std::string &result) {
    auto spacePos = result.find(" ");
    result.insert(spacePos + 1, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ");
    result += " .";
}
