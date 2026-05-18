#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#define CLEAR_SCREEN system("cls")
#else
#define CLEAR_SCREEN system("clear")
#endif

// ==============================================
// 如语言 V4.0 · 万川拟境 · 终极封板版
// 天道已定 · 神码铸成 · 不可再改
// ==============================================
class WanchuanCore {
public:
    std::array<uint8_t, 10> reg{0};
    std::array<uint8_t, 128> zhouTianRing{0};
    std::array<uint8_t, 60>  jiaZiRing{0};
    std::array<uint8_t, 64>  visualSRAM{0};

    struct NayinEntry {
        uint8_t nayinCode : 5;
        uint8_t perm      : 3;
    };
    std::array<NayinEntry, 30> nayinMMU;

    uint8_t pc       = 0;
    uint8_t ringPtr  = 0;
    bool    xiheMode = false;
    bool    halt     = false;

    const char* hex_name[64] = {
        "坤", "剥", "比", "观", "豫", "晋", "萃", "否",
        "谦", "艮", "蹇", "渐", "小过", "旅", "咸", "遁",
        "师", "蒙", "坎", "涣", "解", "未济", "困", "讼",
        "升", "蛊", "井", "巽", "恒", "鼎", "大过", "姤",
        "复", "颐", "屯", "益", "震", "噬嗑", "随", "无妄",
        "明夷", "贲", "既济", "家人", "丰", "离", "革", "同人",
        "临", "损", "节", "中孚", "归妹", "睽", "兑", "履",
        "泰", "大畜", "需", "小畜", "大壮", "大有", "夬", "乾"
    };

    const char* stem_name[10] = {"甲","乙","丙","丁","戊","己","庚","辛","壬","癸"};
    const int stem_map[8] = {1, 2, 4, 7, 6, 5, 3, 0};

    WanchuanCore() {
        for (auto& e : nayinMMU) e = {0, 0x7};
        nayinMMU[0] = {0, 0x5};
        jiaZiRing.fill(0);
        visualSRAM.fill(0);
    }

    struct Field {
        uint8_t rippleLvl : 2;
        bool    ringSel   : 1;
        uint8_t reserved  : 2;
        uint8_t dizhi     : 4;
        uint8_t nayin     : 5;
        uint8_t tiangan   : 4;
        uint8_t jiaziPh   : 2;
    };

    Field decodeField(uint32_t fw) {
        return {
            (uint8_t)((fw >> 18) & 3),
            (bool)((fw >> 17) & 1),
            (uint8_t)((fw >> 15) & 3),
            (uint8_t)((fw >> 11) & 15),
            (uint8_t)((fw >> 6) & 31),
            (uint8_t)((fw >> 2) & 15),
            (uint8_t)(fw & 3)
        };
    }

    uint8_t getInnerTrigram(uint8_t hex) { return (hex >> 1) & 7; }
    uint8_t getOuterTrigram(uint8_t hex) { return (hex >> 4) & 7; }

    uint8_t trigramToGan(uint8_t t) {
        switch(t){
            case 0b111: return 0; case 0b000: return 1;
            case 0b001: return 2; case 0b110: return 3;
            case 0b010: return 4; case 0b101: return 5;
            case 0b100: return 6; case 0b011: return 7;
            default: return 0;
        }
    }

    // 【天道铁律】永世焊死
    // 坤=1(STORE),艮=2(LOAD),坎=3(ALU),巽=4(SHIFT),震=5(JUMP),离=6(LOGIC),兑=7(PORT),乾=8(SYS)
    uint8_t trigramToOp(uint8_t t) {
        switch(t){
            case 0b000: return 1; case 0b001: return 2; case 0b010: return 3;
            case 0b011: return 4; case 0b100: return 5; case 0b101: return 6;
            case 0b110: return 7; case 0b111: return 8; default: return 0;
        }
    }

    std::string checkYaoType(uint8_t yao) {
        int cnt = __builtin_popcount(yao);
        if (cnt == 1) return "【一阳爻】";
        if (cnt == 5) return "【一阴爻】";
        if (cnt == 0) return "【全阴坤】";
        if (cnt == 6) return "【全阳乾】";
        return "";
    }

    void printHexagramArt(uint8_t yao) {
        for (int i=5; i>=0; i--) {
            bool yang = (yao >> i) & 1;
            std::cout << "  " << (yang ? "━━━━━━━" : "━━  ━━") << "\n";
        }
    }

    void printStatus(uint8_t hex, uint32_t field) {
        CLEAR_SCREEN;
        Field f = decodeField(field);
        bool yi = hex & 1;
        uint8_t yao = (hex >> 1) & 0x3F;
        uint8_t inner = yao & 7;
        uint8_t outer = (yao >> 3) & 7;
        int idx = stem_map[inner];

        std::cout << "\e[36m========================================\e[0m\n";
        std::cout << "\e[33m  " << hex_name[yao] << " 乄:" << (yi?"有":"无")
                  << " " << checkYaoType(yao) << "\e[0m\n";
        std::cout << "\e[36m========================================\e[0m\n";
        printHexagramArt(yao);
        std::cout << "\e[32m目标: " << stem_name[idx] << "=" << (int)reg[idx]
                  << "  源: " << stem_name[f.tiangan] << "=" << (int)reg[f.tiangan] << "\e[0m\n";
        std::cout << "\e[35m地支:" << (int)f.dizhi
                  << " 纳音:" << (int)f.nayin
                  << " 涟漪:" << (int)f.rippleLvl
                  << " 甲子指针:" << (int)ringPtr << "\e[0m\n";
        std::cout << "\e[36m========================================\e[0m\n";
    }

    void periphWrite(uint8_t d, uint8_t v) {
        if (d == 6)
            std::cout << "\e[32m>>> 午通道输出: [" << (char)v << "] (" << (int)v << ")\e[0m\n";
        if (d == 3) {
            visualSRAM[v & 0x3F] = v;
            std::cout << "\e[35m[写卯内存 0x" << std::hex << (int)v << std::dec << "]\e[0m\n";
        }
    }

    uint8_t periphRead(uint8_t d) {
        if (d == 7) {
            std::cout << "\e[33m输入字符: \e[0m";
            char c; std::cin >> c; std::cin.ignore();
            return (uint8_t)c;
        }
        return 0;
    }

    // ==============================================
    // 执行核心 · 太极ALU永世不变
    // ==============================================
    void execute(uint8_t hex, uint32_t field) {
        Field f      = decodeField(field);
        bool yi      = hex & 1;
        uint8_t inner = getInnerTrigram(hex);
        uint8_t outer = getOuterTrigram(hex);
        uint8_t tg    = trigramToGan(inner);
        uint8_t op    = trigramToOp(outer);
        uint8_t& dst  = reg[tg];
        uint8_t src   = reg[f.tiangan];

        uint32_t addr;
        uint8_t mem_val = 0;
        if (!f.ringSel) {
            addr = (f.dizhi * 8 + (reg[5] & 7)) % 128;
            mem_val = zhouTianRing[addr];
        } else {
            addr = (ringPtr + f.dizhi) % 60;
            mem_val = jiaZiRing[addr];
        }

        bool isWrite = (op == 1 || op == 3 || op == 4 || op == 6 || op == 7 || op == 8);
        if (f.nayin < 30 && isWrite && !(nayinMMU[f.nayin].perm & 2)) {
            std::cout << "\e[31m!!! 纳音违例：海中金只读，三级涟漪！\e[0m\n";
            f.rippleLvl = 2;
        }

        if (!yi) { pc = (pc+1)&127; return; }
        printStatus(hex, field);

        switch(op) {
            case 1: {
                if (!f.ringSel) zhouTianRing[addr] = dst;
                else            jiaZiRing[addr]    = dst;
                break;
            }
            case 2: { dst = mem_val;            break; }
            case 3: { dst = reg[0] + src;        break; } // 太极ALU：甲+源
            case 4: { dst = reg[0] << (src&7);   break; }
            case 5: { pc = mem_val & 127; reg[5]=pc; return; }
            case 6: { dst = reg[0] & src;        break; }
            case 7: {
                if (f.dizhi == 6 || f.dizhi == 3) periphWrite(f.dizhi, dst);
                if (f.dizhi == 7) dst = periphRead(f.dizhi);
                break;
            }
            case 8: { xiheMode = (src == 0x55); break; }
            default: break;
        }

        switch(f.rippleLvl) {
            case 0: pc = (pc+1)&127; break;
            case 1: jiaZiRing[(ringPtr+1)%60] = dst; pc = (pc+1)&127; break;
            case 2:
                std::cout << "\e[31m=== 三级涟漪：系统复位 ===\e[0m\n";
                pc = ringPtr = 0; reg.fill(0); halt = false;
                for (auto& e : nayinMMU) e.perm = 0x7;
                return;
        }
        reg[5] = pc;
    }

    void setProgram(const std::vector<uint8_t>& hexes, const std::vector<uint32_t>& fields) {
        for (size_t i=0; i<hexes.size() && i<128; i++) {
            zhouTianRing[i] = hexes[i];
            fieldWords[i] = fields[i];
        }
    }

    void run() {
        while (!halt && pc < 128 && zhouTianRing[pc] != 0) {
            execute(zhouTianRing[pc], fieldWords[pc]);
            std::cout << ">> 回车继续\n";
            std::cin.get();
        }
        std::cout << "\n=== 万川拟境 停机 ===\n";
    }

private:
    std::array<uint32_t, 128> fieldWords{0};
};

// ===================== 创世测试 =====================
void test_ChuangShi(WanchuanCore& cpu) {
    std::cout << "\n\e[1m【创世脉冲】甲(42) → 午通道输出*\e[0m\n";
    cpu.reg[0] = 42;
    cpu.setProgram({0x6F}, {(0<<18)|(0<<17)|(6<<11)|(1<<6)|(0<<2)});
    cpu.run();
}

// ===================== 纳音违例测试 =====================
void test_JinZhiNu(WanchuanCore& cpu) {
    std::cout << "\n\e[1m【金之怒】海中金只读违例\e[0m\n";
    cpu.setProgram({0x0B}, {(0<<18)|(0<<17)|(3<<11)|(0<<6)|(5<<2)});
    cpu.run();
}

// ===================== 终极斐波那契 · 封箱之作 =====================
void test_LunHui(WanchuanCore& cpu) {
    std::cout << "\n\e[1m【轮回之环】太极ALU斐波那契流转\e[0m\n";
    cpu.reg[0] = 1; cpu.reg[1] = 1;

    uint32_t f_alu    = (0<<18)|(0<<17)|(0<<11)|(1<<6)|(1<<2);
    uint32_t f_st_jia = (0<<18)|(0<<17)|(0<<11)|(1<<6)|(0<<2);
    uint32_t f_ld_yi  = (0<<18)|(0<<17)|(0<<11)|(1<<6)|(1<<2);
    uint32_t f_st_ji  = (0<<18)|(0<<17)|(1<<11)|(1<<6)|(5<<2);
    uint32_t f_ld_jia = (0<<18)|(0<<17)|(1<<11)|(1<<6)|(0<<2);
    uint32_t f_out    = (0<<18)|(0<<17)|(6<<11)|(1<<6)|(0<<2);

    std::vector<uint8_t> hexes = {
        0x2B,0x0F,0x11,0x0B,0x1F,0x6F,
        0x2B,0x0F,0x11,0x0B,0x1F,0x6F,
        0x2B,0x0F,0x11,0x0B,0x1F,0x6F
    };
    std::vector<uint32_t> fields = {
        f_alu,f_st_jia,f_ld_yi,f_st_ji,f_ld_jia,f_out,
        f_alu,f_st_jia,f_ld_yi,f_st_ji,f_ld_jia,f_out,
        f_alu,f_st_jia,f_ld_yi,f_st_ji,f_ld_jia,f_out
    };
    cpu.setProgram(hexes, fields);
    std::cout << "预期输出：2 → 3 → 5\n";
    cpu.run();
}

// ===================== 天地气交 · 回显 =====================
void test_QiJiao(WanchuanCore& cpu) {
    std::cout << "\n\e[1m【天地气交】午未回显\e[0m\n";
    uint32_t f_in  = (0<<18)|(0<<17)|(7<<11)|(1<<6)|(0<<2);
    uint32_t f_out = (0<<18)|(0<<17)|(6<<11)|(1<<6)|(0<<2);
    cpu.setProgram({0x6F,0x6F},{f_in,f_out});
    cpu.run();
}

// ==============================================
// 主函数 · 万川降临
// ==============================================
int main() {
    CLEAR_SCREEN;
    std::cout << "========================================" << std::endl;
    std::cout << "       万川拟境 · 如语言 V4.0 终版       " << std::endl;
    std::cout << "         卦象即指令 · 干支即总线         " << std::endl;
    std::cout << "========================================" << std::endl;

    WanchuanCore c1; test_ChuangShi(c1);
    WanchuanCore c2; test_JinZhiNu(c2);
    WanchuanCore c3; test_LunHui(c3);
    WanchuanCore c4; test_QiJiao(c4);

    std::cout << "\n\e[32m天道已成 · 万川归流 · 封板定鼎！\e[0m" << std::endl;
    return 0;
}