// 测试：午未UART回显
// 编译：g++ -std=c++11 -o test_uart test_uart.cpp

#include <iostream>
#include "../src/wanchuan_v4_final.cpp"

int main() {
    WanchuanCore cpu;

    // 设置纳音MMU
    cpu.nayinMMU[1] = {1, 1, 0x20, 4, 0x7, 3, 1, 1, true, true}; // 炉中火UART

    std::cout << "=== 午未UART回显测试 ===" << std::endl;
    std::cout << "请输入字符（按Enter发送）：" << std::endl;

    char ch;
    std::cin >> ch;
    cpu.reg[3] = ch; // 丁 = 输入字符

    // 场域字：炉中火，1级涟漪，目标丁
    uint32_t field = (0 << 18) | (1 << 17) | (6 << 11) | (1 << 6) | (3 << 2);

    // 履乄：读取输入
    cpu.execute(0x21, field);

    // 大有乄：输出回显
    cpu.execute(0x08, field);

    return 0;
}
