#ifndef NET_FETCHER_H
#define NET_FETCHER_H

#include <string>
#include <vector>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>

// Network result structure
struct FetchResult {
    std::string url;
    std::string html;
    int statusCode;
    bool success;
    std::string error;
};

// HTTP headers
struct HttpHeader {
    std::string name;
    std::string value;
};

class NetFetcher {
public:
    NetFetcher();
    ~NetFetcher();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Connectivity
    bool IsOnline();
    bool CheckWiFiConnection();
    std::string GetConnectionType();
    
    // HTTP operations
    FetchResult FetchURL(const std::string& url, int timeoutSec = 30);
    FetchResult FetchWithHeaders(const std::string& url, 
                                 const std::vector<HttpHeader>& headers,
                                 int timeoutSec = 30);
    
    // Batch operations
    std::vector<FetchResult> FetchMultiple(const std::vector<std::string>& urls,
                                          int maxConcurrent = 3);
    
    // Settings
    void SetUserAgent(const std::string& ua);
    void SetTimeout(int seconds);
    void SetMaxRetries(int retries);
    
    // Status
    bool IsInitialized() const { return initialized; }
    int GetLastError() const { return lastError; }
    
private:
    bool initialized;
    int netMemId;
    int httpMemId;
    int lastError;
    
    std::string userAgent;
    int timeoutSeconds;
    int maxRetries;
    
    // HTTP helpers
    int CreateHTTPTemplate(const std::string& url, const char* method);
    bool ReadResponse(int connId, std::string& outHtml);
    void CleanupHTTP(int tmplId, int connId, int reqId);
    
    // Network module management
    bool InitNetModules();
    void ShutdownNetModules();
};

#endif // NET_FETCHER_H
