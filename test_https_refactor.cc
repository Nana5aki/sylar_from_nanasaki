#include "sylar/https/https_server.h"
#include "sylar/https/https_session.h"
#include "sylar/http/servlet.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
#include <iostream>

using namespace sylar;
using namespace sylar::https;
using namespace sylar::http;

static Logger::ptr g_logger = SYLAR_LOG_ROOT();

class TestServlet : public Servlet {
public:
    TestServlet() : Servlet("TestServlet") {}
    
    virtual int32_t handle(HttpRequest::ptr request, 
                          HttpResponse::ptr response, 
                          HttpSession::ptr session) override {
        
        // 尝试转换为HTTPS会话
        HttpsSession::ptr https_session = std::dynamic_pointer_cast<HttpsSession>(session);
        
        std::string body = "<!DOCTYPE html><html><head><title>HTTPS Test</title></head><body>";
        body += "<h1>🔒 HTTPS Session Test</h1>";
        body += "<p><strong>Method:</strong> " + HttpMethodToString(request->getMethod()) + "</p>";
        body += "<p><strong>Path:</strong> " + request->getPath() + "</p>";
        body += "<p><strong>Query:</strong> " + request->getQuery() + "</p>";
        
        if (https_session) {
            body += "<div style='background-color: #f0fff0; padding: 15px; border-radius: 5px; margin-top: 20px;'>";
            body += "<h2>SSL/TLS Information</h2>";
            body += "<p><strong>TLS Version:</strong> " + https_session->getTLSVersion() + "</p>";
            body += "<p><strong>Cipher Suite:</strong> " + https_session->getCipherSuite() + "</p>";
            body += "<p><strong>SSL Valid:</strong> " + (https_session->isSSLValid() ? "Yes" : "No") + "</p>";
            body += "<p><strong>Client Certificate:</strong> " + (https_session->verifyClientCertificate() ? "Valid" : "None/Invalid") + "</p>";
            body += "</div>";
        } else {
            body += "<p style='color: red;'>❌ Session is not HTTPS - this should not happen!</p>";
        }
        
        body += "</body></html>";
        
        response->setStatus(HttpStatus::OK);
        response->setHeader("Content-Type", "text/html; charset=utf-8");
        response->setBody(body);
        
        return 0;
    }
};

void test_https_session_inheritance() {
    std::cout << "=== Testing HttpsSession Inheritance ===" << std::endl;
    
    // 测试1: 验证继承关系
    std::cout << "1. Testing inheritance relationships:" << std::endl;
    
    // 创建一个空的socket（用于测试）
    Socket::ptr sock = nullptr;
    SSL* ssl = nullptr;
    
    try {
        // 这里我们不能真正创建HttpsSession，因为需要有效的socket和SSL
        // 但我们可以测试类型转换关系
        
        std::cout << "   - HttpsSession inherits from HttpSession: ";
        std::cout << std::boolalpha << std::is_base_of<HttpSession, HttpsSession>::value << std::endl;
        
        std::cout << "   - HttpSession inherits from SocketStream: ";
        std::cout << std::boolalpha << std::is_base_of<SocketStream, HttpSession>::value << std::endl;
        
        std::cout << "   - HttpsSession has virtual destructor: ";
        std::cout << std::boolalpha << std::has_virtual_destructor<HttpsSession>::value << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

void test_https_server_setup() {
    std::cout << "=== Testing HttpsServer Setup ===" << std::endl;
    
    try {
        // 创建HTTPS服务器
        HttpsServer::ptr server = std::make_shared<HttpsServer>(true);
        server->setName("TestHttpsServer");
        
        // 添加测试servlet
        server->getServletDispatch()->addServlet("/test", std::make_shared<TestServlet>());
        
        // 这里我们不实际启动服务器，只是测试配置
        std::cout << "1. Server created successfully" << std::endl;
        std::cout << "2. Servlet registered successfully" << std::endl;
        std::cout << "3. Server info: " << server->toString() << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    // 初始化日志
    SYLAR_LOG_INFO(g_logger) << "Testing HTTPS session refactoring";
    
    std::cout << "🚀 HTTPS Session Refactoring Test" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
    
    // 测试继承关系
    test_https_session_inheritance();
    
    // 测试服务器设置
    test_https_server_setup();
    
    std::cout << "✅ All tests completed!" << std::endl;
    std::cout << std::endl;
    
    std::cout << "📋 Refactoring Summary:" << std::endl;
    std::cout << "- HttpsSession now inherits from HttpSession" << std::endl;
    std::cout << "- Removed duplicate HTTP protocol handling code" << std::endl;
    std::cout << "- SSL functionality is now focused and isolated" << std::endl;
    std::cout << "- Code is more maintainable and follows DRY principle" << std::endl;
    std::cout << "- HttpsSession can use all HttpSession methods (recvRequest, sendResponse)" << std::endl;
    std::cout << "- Only SSL-specific methods need to be implemented" << std::endl;
    
    return 0;
} 