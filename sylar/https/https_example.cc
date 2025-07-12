/*
 * @Author: Nana5aki
 * @Date: 2025-01-02 10:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-01-02 10:00:00
 * @FilePath: /sylar_from_nanasaki/sylar/https/https_example.cc
 */
#include "https_server.h"
#include "https_connection.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
#include "sylar/address.h"
#include "sylar/http/servlet.h"
#include <iostream>

namespace sylar {
namespace https {

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

// 简单的HTTPS服务器示例
class SimpleHttpsServlet : public sylar::http::Servlet {
public:
    SimpleHttpsServlet() : Servlet("SimpleHttpsServlet") {}
    
    virtual int32_t handle(sylar::http::HttpRequest::ptr request, 
                          sylar::http::HttpResponse::ptr response, 
                          sylar::http::HttpSession::ptr session) override {
        
        // 尝试转换为HTTPS会话以获取SSL信息
        HttpsSession::ptr https_session = std::dynamic_pointer_cast<HttpsSession>(session);
        
        std::string body = R"(
<!DOCTYPE html>
<html>
<head>
    <title>HTTPS Server Example</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .info { background-color: #f0f8ff; padding: 20px; border-radius: 5px; }
        .ssl-info { background-color: #f0fff0; padding: 15px; border-radius: 5px; margin-top: 20px; }
    </style>
</head>
<body>
    <h1>🔒 HTTPS Server Example</h1>
    <div class="info">
        <h2>请求信息</h2>
        <p><strong>方法:</strong> )" + sylar::http::HttpMethodToString(request->getMethod()) + R"(</p>
        <p><strong>路径:</strong> )" + request->getPath() + R"(</p>
        <p><strong>查询:</strong> )" + request->getQuery() + R"(</p>
        <p><strong>用户代理:</strong> )" + request->getHeader("User-Agent") + R"(</p>
        <p><strong>主机:</strong> )" + request->getHeader("Host") + R"(</p>
    </div>
)";

        if (https_session) {
            body += R"(
    <div class="ssl-info">
        <h2>SSL/TLS 信息</h2>
        <p><strong>TLS版本:</strong> )" + https_session->getTLSVersion() + R"(</p>
        <p><strong>密码套件:</strong> )" + https_session->getCipherSuite() + R"(</p>
        <p><strong>客户端证书验证:</strong> )" + (https_session->verifyClientCertificate() ? "通过" : "未通过/无证书") + R"(</p>
    </div>
)";
        }

        body += R"(
    <div style="margin-top: 20px;">
        <h2>测试链接</h2>
        <p><a href="/test">测试页面</a></p>
        <p><a href="/info">服务器信息</a></p>
    </div>
</body>
</html>
)";

        response->setStatus(sylar::http::HttpStatus::OK);
        response->setHeader("Content-Type", "text/html; charset=utf-8");
        response->setBody(body);
        return 0;
    }
};

class TestHttpsServlet : public sylar::http::Servlet {
public:
    TestHttpsServlet() : Servlet("TestHttpsServlet") {}
    
    virtual int32_t handle(sylar::http::HttpRequest::ptr request, 
                          sylar::http::HttpResponse::ptr response, 
                          sylar::http::HttpSession::ptr session) override {
        
        std::string body = R"(
<!DOCTYPE html>
<html>
<head>
    <title>HTTPS Test Page</title>
</head>
<body>
    <h1>🔒 HTTPS 测试页面</h1>
    <p>如果您看到这个页面，说明HTTPS服务器工作正常！</p>
    <p>连接是安全的，数据已加密传输。</p>
    <p><a href="/">返回首页</a></p>
</body>
</html>
)";

        response->setStatus(sylar::http::HttpStatus::OK);
        response->setHeader("Content-Type", "text/html; charset=utf-8");
        response->setBody(body);
        return 0;
    }
};

// HTTPS服务器示例
void httpsServerExample() {
    sylar::IOManager iom(1, true, "main");
    
    // 创建HTTPS服务器
    HttpsServer::ptr server = std::make_shared<HttpsServer>(true, &iom, &iom, &iom);
    
    // 加载证书（需要先生成证书文件）
    if (!server->loadCertificates("server.crt", "server.key")) {
        SYLAR_LOG_ERROR(g_logger) << "Failed to load certificates. Please generate server.crt and server.key first.";
        SYLAR_LOG_INFO(g_logger) << "Use: openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes";
        return;
    }
    
    // 设置Servlet
    server->getServletDispatch()->addServlet("/", std::make_shared<SimpleHttpsServlet>());
    server->getServletDispatch()->addServlet("/test", std::make_shared<TestHttpsServlet>());
    
    // 绑定地址
    auto addr = sylar::Address::LookupAny("0.0.0.0:8443");
    if (!addr) {
        SYLAR_LOG_ERROR(g_logger) << "Failed to lookup address";
        return;
    }
    
    if (!server->bind(addr)) {
        SYLAR_LOG_ERROR(g_logger) << "Failed to bind address";
        return;
    }
    
    server->setName("HttpsServerExample");
    
    // 启动服务器
    if (!server->start()) {
        SYLAR_LOG_ERROR(g_logger) << "Failed to start server";
        return;
    }
    
    SYLAR_LOG_INFO(g_logger) << "HTTPS Server started on https://localhost:8443";
    SYLAR_LOG_INFO(g_logger) << "Press Ctrl+C to stop the server";
    
    // 运行IO管理器
    iom.start();
}

// HTTPS客户端示例
void httpsClientExample() {
    SYLAR_LOG_INFO(g_logger) << "HTTPS Client Example";
    
    // 发送GET请求
    auto result = HttpsConnection::DoGet("https://httpbin.org/get", 10000);
    if (result && result->result == (int)HttpsResult::Error::OK) {
        SYLAR_LOG_INFO(g_logger) << "GET Request Success:";
        SYLAR_LOG_INFO(g_logger) << "Status: " << result->response->getStatus();
        SYLAR_LOG_INFO(g_logger) << "Body: " << result->response->getBody();
    } else {
        SYLAR_LOG_ERROR(g_logger) << "GET Request Failed: " << (result ? result->error : "null result");
    }
    
    // 发送POST请求
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    
    std::string json_body = R"({"name": "sylar", "version": "1.0", "type": "https_client"})";
    
    auto post_result = HttpsConnection::DoPost("https://httpbin.org/post", 10000, headers, json_body);
    if (post_result && post_result->result == (int)HttpsResult::Error::OK) {
        SYLAR_LOG_INFO(g_logger) << "POST Request Success:";
        SYLAR_LOG_INFO(g_logger) << "Status: " << post_result->response->getStatus();
        SYLAR_LOG_INFO(g_logger) << "Body: " << post_result->response->getBody();
    } else {
        SYLAR_LOG_ERROR(g_logger) << "POST Request Failed: " << (post_result ? post_result->error : "null result");
    }
}

// 使用连接池的示例
void httpsConnectionPoolExample() {
    SYLAR_LOG_INFO(g_logger) << "HTTPS Connection Pool Example";
    
    // 创建连接池
    auto pool = HttpsConnectionPool::Create("https://httpbin.org:443", "", 10, 30 * 1000, 10);
    if (!pool) {
        SYLAR_LOG_ERROR(g_logger) << "Failed to create connection pool";
        return;
    }
    
    // 使用连接池发送请求
    for (int i = 0; i < 5; ++i) {
        auto result = pool->doRequest(sylar::http::HttpMethod::GET, "/get", 10000);
        if (result && result->result == (int)HttpsResult::Error::OK) {
            SYLAR_LOG_INFO(g_logger) << "Pool Request " << i << " Success: Status " << result->response->getStatus();
        } else {
            SYLAR_LOG_ERROR(g_logger) << "Pool Request " << i << " Failed: " << (result ? result->error : "null result");
        }
    }
}

} // namespace https
} // namespace sylar

// 主函数示例
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <server|client|pool>" << std::endl;
        std::cout << "  server: 启动HTTPS服务器示例" << std::endl;
        std::cout << "  client: 运行HTTPS客户端示例" << std::endl;
        std::cout << "  pool:   运行HTTPS连接池示例" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "server") {
        sylar::https::httpsServerExample();
    } else if (mode == "client") {
        sylar::https::httpsClientExample();
    } else if (mode == "pool") {
        sylar::https::httpsConnectionPoolExample();
    } else {
        std::cout << "Unknown mode: " << mode << std::endl;
        return 1;
    }
    
    return 0;
} 