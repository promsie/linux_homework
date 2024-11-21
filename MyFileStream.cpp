#include <iostream>
#include <cstdio> // For FILE, fopen, fclose, etc.
#include <cstring> // For std::memcpy
#include <memory>
#include <stdexcept>

// 自定义 make_unique 函数，用于创建数组类型的 unique_ptr
template<typename T>
std::unique_ptr<T[]> MakeUniqueArray(std::size_t size) {
    return std::unique_ptr<T[]>(new T[size]);
}

// 定义缓存大小的宏，设置为256字节
#define CacheSize 256

class MyFStream {
public:
    // 构造函数，初始化文件名、缓存大小、缓存位置和缓存填充量
    MyFStream(const std::string& filename)
    : filename(filename), cacheSize(CacheSize), cachePos(0), cacheFilled(0), filePos(0), fp(nullptr) {
        // 分配固定大小的缓存内存
        cache = MakeUniqueArray<char>(cacheSize);
    }

    // 打开文件，如果文件不存在则创建
    void Open() {
        if (fp != nullptr) {
            fclose(fp);
        }
        fp = fopen(filename.c_str(), "r+b"); // 修改为 "r+b" 模式
        if (!fp) {
            // 如果文件打开失败，抛出运行时异常
            throw std::runtime_error("Failed to open file: " + filename);
        }
    }

    // 析构函数，确保在对象销毁时关闭文件
    ~MyFStream() {
        Close();  // 调用 Close 方法确保文件被正确关闭
    }

    // 检查文件是否打开
    bool IsOpen() const {
        return fp != nullptr;  // 如果文件指针不为空，则文件已打开
    }

    // 从文件中读取数据到缓冲区
    size_t Read(void* buffer, size_t size) {
        if (!IsOpen()) return 0;  // 如果文件未打开，返回0

        size_t totalRead = 0;  // 记录总读取字节数

        while (totalRead < size) {
            // 如果缓存区中没有数据或不足以满足请求，从文件中填充缓存区
            if (cachePos == cacheFilled) {
                cacheFilled = fread(cache.get(), 1, cacheSize, fp);
                cachePos = 0;  // 重置缓存区位置
                if (cacheFilled == 0) {
                    break;  // 如果文件末尾或错误，跳出循环
                }
                filePos += cacheFilled;  // 更新文件位置
            }

            // 计算缓存区中可读取的字节数
            size_t cachedDataSize = std::min(size - totalRead, cacheFilled - cachePos);
            if (cachedDataSize > 0) {
                // 如果缓存区中有数据，从缓存区读取
                std::memcpy(static_cast<char*>(buffer) + totalRead, cache.get() + cachePos, cachedDataSize);
                cachePos += cachedDataSize;  // 更新缓存区位置
                totalRead += cachedDataSize;  // 更新总读取字节数
            }
        }

        return totalRead;  // 返回总读取字节数
    }

    // 将数据从缓冲区写入文件
    size_t Write(const void* buffer, size_t size) {
        if (!IsOpen()) return 0;  // 如果文件未打开，返回0

        size_t totalWritten = 0;  // 记录总写入字节数

        while (totalWritten < size) {
            // 计算缓存区中剩余的空间
            size_t spaceLeft = cacheSize - cachePos;
            size_t dataToCache = std::min(size - totalWritten, spaceLeft);

            // 将数据从用户缓冲区复制到缓存区
            std::memcpy(cache.get() + cachePos, static_cast<const char*>(buffer) + totalWritten, dataToCache);
            cachePos += dataToCache;  // 更新缓存区位置
            totalWritten += dataToCache;  // 更新总写入字节数

            if (cachePos == cacheSize) {
                // 如果缓存区满，将缓存区数据写入文件
                fwrite(cache.get(), 1, cacheSize, fp);
                filePos += cacheSize;  // 更新文件位置
                cachePos = 0;  // 重置缓存区位置
                cacheFilled = 0;  // 重置缓存区填充量
            }
        }

        return totalWritten;  // 返回总写入字节数
    }

    // 改变文件的读取/写入位置
    void Lseek(size_t offset, int whence) {
        if (!IsOpen()) return;  // 如果文件未打开，直接返回

        if (cachePos > 0) {
            // 如果缓存区中有数据，先写入文件
            fwrite(cache.get(), 1, cachePos, fp);
            cachePos = 0;  // 重置缓存区位置
            cacheFilled = 0;  // 重置缓存区填充量
        }

        // 使用 fseek 更新文件位置
        fseek(fp, offset, whence);
        filePos = ftell(fp);  // 更新文件位置
    }

    // 关闭文件
    void Close() {
        if (IsOpen()) {
            // 将缓存中的数据写入文件
            if (cachePos > 0) {
                fwrite(cache.get(), 1, cachePos, fp);
                cachePos = 0;  // 重置缓存区位置
            }
            // 关闭文件
            fclose(fp);
            fp = nullptr;
        }
    }

private:
    std::string filename;  // 文件名
    size_t cacheSize;  // 缓存大小
    size_t cachePos;  // 当前缓存位置
    size_t cacheFilled;  // 缓存中已填充的数据量
    std::unique_ptr<char[]> cache;  // 缓存内存
    FILE* fp;  // 文件指针
    size_t filePos;  // 当前文件位置
};

int main() {
    const std::string Filename = "example.txt";

    MyFStream File(Filename);
    try {
        File.Open();  // 打开文件，如果文件不存在则创建

        if (File.IsOpen()) {
            const char* InitialMessage = "Hello, World! This message is longer than 256 bytes and will be split into multiple writes.";
            size_t InitialMessageLength = strlen(InitialMessage) + 1;
            File.Write(InitialMessage, InitialMessageLength);  // 写入初始数据

            // 使用 Lseek 将文件指针移动到文件末尾
            File.Lseek(0, SEEK_END);

            // 从文件末尾开始继续写入数据
            const char* AdditionalMessage = " This is additional data appended at the end of the file.";
            size_t AdditionalMessageLength = strlen(AdditionalMessage) + 1;
            File.Write(AdditionalMessage, AdditionalMessageLength);  // 写入追加数据

            File.Close();  // 关闭文件

            // 重新打开文件以读取数据
            File.Open();  // 使用 "r+b" 模式打开文件，允许读写，且不截断文件
            char Buffer[1024];
            File.Lseek(0, SEEK_SET);  // 移动文件指针到开始位置
            size_t BytesRead = File.Read(Buffer, sizeof(Buffer));  // 读取数据
            std::cout << "Read from file: " << std::string(Buffer, BytesRead) << std::endl;  // 输出读取的数据
            File.Close();  // 关闭文件
        } else {
            std::cerr << "Failed to open file: " << Filename << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;  // 捕获并处理异常
    }

    return 0;
}
