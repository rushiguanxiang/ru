// 测试：甲子轮斐波那契生成
// 编译：g++ -std=c++11 -o test_fib test_fibonacci.cpp

#include <iostream>
#include "../src/wanchuan_v4_final.cpp"

int main() {
    WanchuanCore cpu;

    // 设置纳音MMU
    cpu.nayinMMU[2] = {2, 1, 0x30, 8, 0x7, 1, 2, 0, true, true}; // 大林木堆区

    // 初始化
    cpu.reg[0] = 0;   // 甲 = F(0)
    cpu.reg[1] = 1;   // 乙 = F(1)
    cpu.ringPtr = 0;

    std::cout << "=== 甲子轮斐波那契测试 ===" << std::endl;

    // 场域字：大林木，1级涟漪，目标丙
    uint32_t field = (0 << 18) | (1 << 17) | (2 << 11) | (2 << 6) | (2 << 2);

    // 生成8项
    for (int i = 0; i < 8; i++) {
        cpu.execute(0x2B, field); // 既济乄：丙 = 甲 + 乙
        cpu.jiaZiRing[cpu.ringPtr] = cpu.reg[2];
        std::cout << "F(" << i << ")=" << (int)cpu.reg[2] << " ";

        cpu.execute(0x55, field); // 未济乄：甲 = 乙
        cpu.reg[0] = cpu.reg[1];

        cpu.execute(0x2B, field); // 既济乄：乙 = 丙
        cpu.reg[1] = cpu.reg[2];

        cpu.ringPtr = (cpu.ringPtr + 1) % 60;
    }
    std::cout << std::endl;

    return 0;
}
