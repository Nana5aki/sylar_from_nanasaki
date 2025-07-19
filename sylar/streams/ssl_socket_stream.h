/*
 * @Author: Nana5aki
 * @Date: 2025-07-06 16:36:10
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-19 17:13:49
 * @FilePath: /sylar_from_nanasaki/sylar/streams/ssl_socket_stream.h
 * @Description: SSL套接字流头文件 - 提供基于OpenSSL的加密数据传输能力
 *               这是HTTPS模块的基础组件，封装了SSL/TLS协议的底层操作
 */
#pragma once

#include "sylar/streams/sock_stream.h"
#include <memory>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace sylar {

/**
 * @brief SSL套接字流封装类
 * @details 继承自SocketStream，在普通TCP套接字流的基础上添加SSL/TLS加密支持
 *          提供透明的加密数据传输能力，支持客户端和服务端模式
 *          主要用于HTTPS服务器和客户端的底层数据传输
 *
 * 主要功能：
 * - SSL/TLS加密数据传输
 * - SSL握手处理
 * - 证书验证
 * - 错误处理和状态管理
 *
 * 使用场景：
 * - HTTPS服务器端连接处理
 * - HTTPS客户端连接建立
 * - 需要加密传输的网络应用
 */
class SslSocketStream : public SocketStream {
public:
  using ptr = std::shared_ptr<SslSocketStream>;

  /**
   * @brief 构造函数 - 使用已存在的SSL对象创建SSL套接字流
   * @param[in] sock Socket类 - 底层TCP套接字对象
   * @param[in] ssl SSL连接对象 - 已创建的OpenSSL连接对象，可以为nullptr
   * @param[in] owner 是否拥有socket所有权 - true表示析构时会关闭socket
   * @details 当ssl参数不为nullptr时，会自动将SSL对象与socket文件描述符关联
   *          适用于服务端接受连接或客户端已完成初步连接的场景
   */
  SslSocketStream(Socket::ptr sock, SSL* ssl, bool owner = true);

  /**
   * @brief 析构函数
   * @details 自动清理SSL连接和相关资源
   *          如果拥有SSL对象所有权，会先执行SSL_shutdown进行优雅关闭
   *          然后释放SSL对象，最后调用父类析构函数清理socket资源
   */
  ~SslSocketStream();

  /**
   * @brief 从SSL连接读取加密数据并解密
   * @param[out] buffer 待接收数据的内存缓冲区
   * @param[in] length 待接收数据的最大长度
   * @return 读取结果
   *      @retval >0 返回实际接收到的解密后数据长度
   *      @retval =0 SSL连接被远端正常关闭
   *      @retval <0 SSL连接错误或socket错误
   * @details 该方法会自动处理SSL层的解密操作
   *          内部调用SSL_read函数进行加密数据读取和解密
   *          可能的错误情况包括：SSL握手未完成、连接断开、SSL协议错误等
   *          对于SSL_ERROR_WANT_READ/WRITE错误，表示需要更多数据才能完成操作
   */
  virtual int read(void* buffer, size_t length) override;

  /**
   * @brief 从SSL连接读取数据到ByteArray对象
   * @param[out] ba 接收数据的ByteArray智能指针
   * @param[in] length 待接收数据的最大长度
   * @return 读取结果
   *      @retval >0 返回实际接收到的解密后数据长度
   *      @retval =0 SSL连接被远端正常关闭
   *      @retval <0 SSL连接错误或socket错误
   * @details 这是read(void*, size_t)方法的ByteArray版本
   *          先读取到临时缓冲区，然后写入ByteArray对象
   *          适用于需要使用ByteArray进行数据处理的场景
   */
  virtual int read(ByteArray::ptr ba, size_t length) override;

  /**
   * @brief 加密数据并通过SSL连接发送
   * @param[in] buffer 待发送数据的内存缓冲区
   * @param[in] length 待发送数据的长度
   * @return 发送结果
   *      @retval >0 返回实际发送的数据长度
   *      @retval =0 SSL连接被远端关闭
   *      @retval <0 SSL连接错误或socket错误
   * @details 该方法会自动处理SSL层的加密操作
   *          内部调用SSL_write函数进行数据加密和发送
   *          可能的错误情况包括：SSL握手未完成、连接断开、SSL协议错误等
   *          对于SSL_ERROR_WANT_READ/WRITE错误，表示需要更多数据才能完成操作
   */
  virtual int write(const void* buffer, size_t length) override;

  /**
   * @brief 从ByteArray对象读取数据并通过SSL连接发送
   * @param[in] ba 待发送数据的ByteArray智能指针
   * @param[in] length 待发送数据的长度
   * @return 发送结果
   *      @retval >0 返回实际发送的数据长度
   *      @retval =0 SSL连接被远端关闭
   *      @retval <0 SSL连接错误或socket错误
   * @details 这是write(const void*, size_t)方法的ByteArray版本
   *          先从ByteArray对象读取数据到临时缓冲区，然后调用write方法发送
   *          适用于使用ByteArray存储待发送数据的场景
   */
  virtual int write(ByteArray::ptr ba, size_t length) override;

  /**
   * @brief 关闭SSL连接和底层socket
   * @details 先执行SSL_shutdown进行SSL层的优雅关闭
   *          然后调用父类的close方法关闭底层socket连接
   *          这是推荐的SSL连接关闭方式，确保数据完整性
   */
  virtual void close() override;

  /**
   * @brief 获取底层SSL连接对象
   * @return SSL* SSL连接对象指针，可能为nullptr
   * @details 返回当前使用的OpenSSL连接对象
   *          可用于直接调用OpenSSL API进行高级操作
   *          注意：直接操作SSL对象可能影响流的状态，请谨慎使用
   */
  SSL* getSSL() const {
    return m_ssl;
  }

  /**
   * @brief 设置SSL对象所有权
   * @details 控制当前SslSocketStream对象是否拥有SSL对象的管理权限
   *          影响析构函数和close()方法中是否执行SSL_shutdown和SSL_free操作
   */
  void setOwnerSSL(bool owner) {
    m_owner_ssl = owner;
  }

  /**
   * @brief 执行SSL握手过程
   * @return bool 握手是否成功
   *      @retval true 握手成功，可以进行加密数据传输
   *      @retval false 握手失败或需要更多数据
   * @details 执行SSL/TLS握手协议，建立安全连接
   *          内部调用SSL_do_handshake函数完成握手过程
   *          握手过程包括：协议版本协商、密码套件选择、密钥交换、身份验证等
   *          对于非阻塞socket，可能需要多次调用直到握手完成
   */
  bool doHandshake();

  /**
   * @brief 检查SSL连接是否处于有效状态
   * @return bool 连接是否有效
   *      @retval true SSL对象存在且底层socket已连接
   *      @retval false SSL对象不存在或底层socket未连接
   * @details 检查SSL连接的基本有效性
   *          包括SSL对象是否存在以及底层socket是否仍然连接
   *          不检查SSL握手是否完成，仅检查基础连接状态
   */
  bool isValid() const;

protected:
  /// SSL连接对象
  SSL* m_ssl;
  /// 是否拥有SSL对象所有权
  bool m_owner_ssl;
};

/**
 * @brief SSL上下文管理器类
 * @details SSL_CTX的C++封装，负责管理SSL/TLS的全局配置
 *          一个SslContext对象可以创建多个SSL连接，共享相同的配置
 *          包括证书、私钥、验证模式、密码套件等安全参数
 *
 * 主要功能：
 * - SSL/TLS协议配置管理
 * - 证书和私钥管理
 * - 客户端/服务端验证模式设置
 * - SSL连接对象创建
 * - 安全参数配置（密码套件、协议版本等）
 *
 * 使用场景：
 * - HTTPS服务器证书配置
 * - HTTPS客户端验证配置
 * - SSL连接池的上下文管理
 */
class SslContext {
public:
  using ptr = std::shared_ptr<SslContext>;

  /**
   * @brief 构造函数 - 创建SSL上下文对象
   * @param[in] method SSL方法指针，指定使用的SSL/TLS协议版本
   *                   nullptr时默认使用TLS_method()（支持所有TLS版本）
   * @details 创建底层SSL_CTX对象并进行基本配置
   *          自动禁用不安全的SSLv2和SSLv3协议
   *          启用SSL_MODE_AUTO_RETRY模式以处理重新协商
   * @throws std::runtime_error 当SSL_CTX创建失败时抛出异常
   */
  SslContext(const SSL_METHOD* method = nullptr);

  /**
   * @brief 析构函数
   * @details 自动释放底层SSL_CTX对象和相关资源
   *          确保SSL上下文对象的安全清理
   */
  virtual ~SslContext();

  /**
   * @brief 静态工厂方法 - 创建SSL上下文智能指针
   * @param[in] method SSL方法指针，指定使用的SSL/TLS协议版本
   * @return SslContext::ptr SSL上下文智能指针
   * @details 推荐使用这个方法创建SslContext对象
   *          返回智能指针确保资源的自动管理
   *          内部调用构造函数创建对象
   */
  static SslContext::ptr Create(const SSL_METHOD* method = nullptr);

  /**
   * @brief 获取底层SSL_CTX对象
   * @return SSL_CTX* SSL上下文指针
   * @details 返回OpenSSL的SSL_CTX对象，用于直接调用OpenSSL API
   *          注意：直接操作SSL_CTX可能影响上下文状态，请谨慎使用
   */
  SSL_CTX* getCtx() const {
    return m_ctx;
  }

  /**
   * @brief 加载SSL证书和私钥文件
   * @param[in] cert_file 证书文件路径（PEM格式）
   * @param[in] key_file 私钥文件路径（PEM格式）
   * @return bool 加载是否成功
   *      @retval true 证书和私钥加载成功且匹配
   *      @retval false 文件不存在、格式错误或私钥与证书不匹配
   * @details 加载服务端SSL证书和私钥文件
   *          支持PEM格式的证书和私钥文件
   *          会自动验证私钥与证书是否匹配
   *          加载成功后该上下文创建的SSL连接都将使用这些证书
   */
  bool loadCertificates(const std::string& cert_file, const std::string& key_file);

  /**
   * @brief 设置SSL证书验证模式
   * @param[in] mode 验证模式，OpenSSL定义的验证标志
   *                 SSL_VERIFY_NONE - 不验证对端证书
   *                 SSL_VERIFY_PEER - 验证对端证书
   *                 SSL_VERIFY_FAIL_IF_NO_PEER_CERT - 要求对端必须提供证书
   *                 SSL_VERIFY_CLIENT_ONCE - 仅在初次握手时验证客户端证书
   * @details 设置SSL连接的证书验证策略
   *          对于服务端，通常设置为SSL_VERIFY_NONE或SSL_VERIFY_PEER
   *          对于客户端，通常设置为SSL_VERIFY_PEER以验证服务器证书
   */
  void setVerifyMode(int mode);

  /**
   * @brief 设置证书验证回调函数
   * @param[in] callback 证书验证回调函数指针
   *                     int callback(int preverify_ok, X509_STORE_CTX *ctx)
   *                     preverify_ok: OpenSSL预验证结果
   *                     ctx: 证书存储上下文，包含证书链信息
   *                     返回值：1表示验证通过，0表示验证失败
   * @details 设置自定义的证书验证逻辑
   *          可以在回调函数中实现额外的证书检查
   *          如证书吊销列表检查、自定义CA验证等
   */
  void setVerifyCallback(int (*callback)(int, X509_STORE_CTX*));

  /**
   * @brief 设置私钥密码回调函数
   * @param[in] callback 密码回调函数指针
   *                     int callback(char *buf, int size, int rwflag, void *userdata)
   *                     buf: 存储密码的缓冲区
   *                     size: 缓冲区大小
   *                     rwflag: 读写标志（0=读，1=写）
   *                     userdata: 用户数据指针
   *                     返回值：密码长度
   * @details 当私钥文件受密码保护时，通过此回调函数提供密码
   *          可以从配置文件、环境变量或用户输入获取密码
   *          增强私钥文件的安全性
   */
  void setPasswordCallback(int (*callback)(char*, int, int, void*));

  /**
   * @brief 创建新的SSL连接对象
   * @return SSL* 新创建的SSL连接对象指针，失败时返回nullptr
   * @details 基于当前上下文配置创建新的SSL连接对象
   *          新创建的SSL对象继承上下文的所有配置
   *          包括证书、验证模式、密码套件等
   *          调用方负责释放返回的SSL对象
   */
  SSL* createSSL();

private:
  /// SSL上下文
  SSL_CTX* m_ctx;
};

}   // namespace sylar