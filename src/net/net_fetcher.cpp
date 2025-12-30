#include "net_fetcher.h"
#include <psp2/sysmodule.h>
#include <psp2/kernel/threadmgr.h>
#include <cstring>
#include <algorithm>

NetFetcher::NetFetcher() : initialized(false), netMemId(-1), httpMemId(-1),
                           lastError(0), timeoutSeconds(30), maxRetries(3) {
    userAgent = "VitaSurvivalAI/1.0 (PS Vita; Educational/Research)";
}

NetFetcher::~NetFetcher() {
    Shutdown();
}

bool NetFetcher::Initialize() {
    if (initialized) return true;
    
    if (!InitNetModules()) {
        return false;
    }
    
    initialized = true;
    return true;
}

void NetFetcher::Shutdown() {
    if (initialized) {
        ShutdownNetModules();
        initialized = false;
    }
}

bool NetFetcher::InitNetModules() {
    // Load network modules
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
    
    // Initialize network
    SceNetInitParam netParam;
    netParam.memory = malloc(1024 * 1024);  // 1MB
    netParam.size = 1024 * 1024;
    netParam.flags = 0;
    
    int ret = sceNetInit(&netParam);
    if (ret < 0) {
        free(netParam.memory);
        lastError = ret;
        return false;
    }
    
    // Initialize NetCtl
    ret = sceNetCtlInit();
    if (ret < 0) {
        lastError = ret;
        return false;
    }
    
    // Initialize HTTP
    ret = sceHttpInit(1024 * 1024);  // 1MB pool
    if (ret < 0) {
        lastError = ret;
        return false;
    }
    
    return true;
}

void NetFetcher::ShutdownNetModules() {
    sceHttpTerm();
    sceNetCtlTerm();
    sceNetTerm();
    
    sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTPS);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

bool NetFetcher::IsOnline() {
    if (!initialized) return false;
    return CheckWiFiConnection();
}

bool NetFetcher::CheckWiFiConnection() {
    SceNetCtlInfo info;
    int ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info);
    return (ret >= 0);
}

std::string NetFetcher::GetConnectionType() {
    if (!IsOnline()) return "offline";
    
    SceNetCtlInfo info;
    int ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_SSID, &info);
    if (ret >= 0) {
        return "WiFi: " + std::string(info.ssid);
    }
    return "connected";
}

FetchResult NetFetcher::FetchURL(const std::string& url, int timeoutSec) {
    FetchResult result;
    result.url = url;
    result.success = false;
    result.statusCode = 0;
    
    if (!initialized) {
        result.error = "Network not initialized";
        return result;
    }
    
    int tmplId = -1, connId = -1, reqId = -1;
    
    // Create HTTP template
    tmplId = sceHttpCreateTemplate(userAgent.c_str(), SCE_HTTP_VERSION_1_1, SCE_TRUE);
    if (tmplId < 0) {
        result.error = "Failed to create HTTP template";
        lastError = tmplId;
        return result;
    }
    
    // Create connection
    connId = sceHttpCreateConnectionWithURL(tmplId, url.c_str(), SCE_FALSE);
    if (connId < 0) {
        result.error = "Failed to create connection";
        lastError = connId;
        CleanupHTTP(tmplId, -1, -1);
        return result;
    }
    
    // Create request
    reqId = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_GET, url.c_str(), 0);
    if (reqId < 0) {
        result.error = "Failed to create request";
        lastError = reqId;
        CleanupHTTP(tmplId, connId, -1);
        return result;
    }
    
    // Set timeout
    sceHttpSetRequestContentLength(reqId, 0);
    sceHttpSetConnectTimeOut(connId, timeoutSec * 1000000);  // microseconds
    sceHttpSetSendTimeOut(reqId, timeoutSec * 1000000);
    sceHttpSetRecvTimeOut(reqId, timeoutSec * 1000000);
    
    // Send request
    int ret = sceHttpSendRequest(reqId, NULL, 0);
    if (ret < 0) {
        result.error = "Failed to send request";
        lastError = ret;
        CleanupHTTP(tmplId, connId, reqId);
        return result;
    }
    
    // Get status code
    ret = sceHttpGetStatusCode(reqId, &result.statusCode);
    if (ret < 0) {
        result.error = "Failed to get status code";
        lastError = ret;
        CleanupHTTP(tmplId, connId, reqId);
        return result;
    }
    
    // Check for success status
    if (result.statusCode < 200 || result.statusCode >= 300) {
        result.error = "HTTP error: " + std::to_string(result.statusCode);
        CleanupHTTP(tmplId, connId, reqId);
        return result;
    }
    
    // Read response
    if (!ReadResponse(reqId, result.html)) {
        result.error = "Failed to read response";
        CleanupHTTP(tmplId, connId, reqId);
        return result;
    }
    
    result.success = true;
    CleanupHTTP(tmplId, connId, reqId);
    
    return result;
}

bool NetFetcher::ReadResponse(int reqId, std::string& outHtml) {
    const int BUFFER_SIZE = 16384;  // 16KB chunks
    char buffer[BUFFER_SIZE];
    outHtml.clear();
    
    int totalRead = 0;
    int maxSize = 5 * 1024 * 1024;  // 5MB max
    
    while (totalRead < maxSize) {
        int read = sceHttpReadData(reqId, buffer, BUFFER_SIZE);
        
        if (read < 0) {
            lastError = read;
            return false;
        }
        
        if (read == 0) {
            break;  // End of data
        }
        
        outHtml.append(buffer, read);
        totalRead += read;
    }
    
    return true;
}

void NetFetcher::CleanupHTTP(int tmplId, int connId, int reqId) {
    if (reqId >= 0) sceHttpDeleteRequest(reqId);
    if (connId >= 0) sceHttpDeleteConnection(connId);
    if (tmplId >= 0) sceHttpDeleteTemplate(tmplId);
}

FetchResult NetFetcher::FetchWithHeaders(const std::string& url,
                                         const std::vector<HttpHeader>& headers,
                                         int timeoutSec) {
    // Similar to FetchURL but with custom headers
    // Implementation would add headers before sending request
    // For now, just use FetchURL
    return FetchURL(url, timeoutSec);
}

std::vector<FetchResult> NetFetcher::FetchMultiple(const std::vector<std::string>& urls,
                                                    int maxConcurrent) {
    std::vector<FetchResult> results;
    
    // For now, fetch sequentially (concurrent fetching is complex on Vita)
    for (const auto& url : urls) {
        results.push_back(FetchURL(url));
        
        // Small delay between requests (rate limiting)
        sceKernelDelayThread(1000000);  // 1 second
    }
    
    return results;
}

void NetFetcher::SetUserAgent(const std::string& ua) {
    userAgent = ua;
}

void NetFetcher::SetTimeout(int seconds) {
    timeoutSeconds = seconds;
}

void NetFetcher::SetMaxRetries(int retries) {
    maxRetries = retries;
}
