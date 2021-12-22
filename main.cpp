#include <iostream>
#include <algorithm>
#include <cstdio>
#include <memory.h>
#include <cstring>
#include <vector>

using namespace std;

using ll = long long;

const short MAX_BUFFER_SIZE = 1024;

struct Forest {
    ll freq;
    short root, symbol;
};

struct Tree {
    short left, right, parent, symbol;
};

vector<bool> bits;

void dfs(short v, Tree tree[], vector<bool> enc[]) {
    if (tree[v].left == tree[v].right && tree[v].left == -1) {
        for (auto bit: bits) {
            enc[tree[v].symbol].push_back(bit);
        }
        return;
    }
    if (tree[v].left != -1) {
        bits.push_back(false);
        dfs(tree[v].left, tree, enc);
        bits.pop_back();
    }
    if (tree[v].right != -1) {
        bits.push_back(true);
        dfs(tree[v].right, tree, enc);
        bits.pop_back();
    }
}

void find_min(Forest forest[], short forest_size, short &min_pos1, short &min_pos2) {
    min_pos1 = min_pos2 = -1;
    for (short i = 0; i < forest_size; i++) {
        if (min_pos1 == -1 || forest[i].freq <= forest[min_pos1].freq) {
            min_pos1 = i;
        }
    }
    for (short i = 0; i < forest_size; i++) {
        if (i == min_pos1) {
            continue;
        }
        if (min_pos2 == -1 || forest[i].freq <= forest[min_pos2].freq) {
            min_pos2 = i;
        }
    }
}

int32_t main(int argc, char *argv[]) {
    if (!strcmp("encode", argv[1])) {
        FILE *INPUT_FILE;
        FILE *OUTPUT_FILE;

        INPUT_FILE = fopen(argv[3], "rb");
        OUTPUT_FILE = fopen(argv[2], "wb");

        ll frequency[256];
        int byte_count = 0;
        memset(frequency, 0LL, sizeof(frequency));

        unsigned char c;
        while (fscanf(INPUT_FILE, "%c", &c) != EOF) {
            byte_count++;
            frequency[c]++;
        }

        Forest forest[256];
        short size_forest = 0;

        for (short i = 0; i < 256; i++) {
            if (frequency[i] != 0) {
                forest[size_forest].freq = frequency[i];
                forest[size_forest].root = size_forest;
                forest[size_forest].symbol = i;
                size_forest++;
            }
        }

        Tree tree[2049];
        for (short i = 0; i < size_forest; i++) {
            tree[i].left = tree[i].right = tree[i].parent = -1;
            tree[i].symbol = forest[i].symbol;
        }

        short min_p1, min_p2, size_tree = size_forest, size_forest_copy = size_forest;
        while (size_forest > 1) {
            find_min(forest, size_forest, min_p1, min_p2);

            tree[size_tree].left = forest[min_p1].root;
            tree[size_tree].right = forest[min_p2].root;
            tree[size_tree].parent = tree[size_tree].symbol = -1;

            tree[tree[size_tree].right].parent = tree[tree[size_tree].left].parent = size_tree;

            forest[min_p1].freq = forest[min_p1].freq + forest[min_p2].freq;
            forest[min_p1].root = size_tree;

            forest[min_p2] = forest[size_forest - 1];

            size_forest--;
            size_tree++;
        }

        vector<bool> encoding[256];
        INPUT_FILE = fopen(argv[3], "rb");
        short written_bits = 0;
        char x = 0;
        dfs(size_tree - 1, tree, encoding);

        size_forest_copy--;
        if (byte_count <= 255) {
            size_tree |= (1 << 14);
        } else if (byte_count <= 32767) {
            size_tree |= (1 << 15);
        }
        fwrite(&size_tree, sizeof(short), 1, OUTPUT_FILE);
        if (byte_count <= 255) {
            size_tree ^= (1 << 14);
        } else if (byte_count <= 65535) {
            size_tree ^= (1 << 15);
        }
        char sfc = (size_forest_copy - 128);
        fwrite(&sfc, sizeof(char), 1, OUTPUT_FILE);
        if (byte_count <= 255) {
            unsigned char k = byte_count;
            fwrite(&k, sizeof(k), 1, OUTPUT_FILE);
        } else if (byte_count <= 65535) {
            unsigned short btcnt = byte_count;
            fwrite(&btcnt, sizeof(short), 1, OUTPUT_FILE);
        } else {
            fwrite(&byte_count, sizeof(int), 1, OUTPUT_FILE);
        }

        for (short i = 0; i < size_tree; i++) {
            if (tree[i].parent == -1) {
                tree[i].parent++;
            } else {
                short p = tree[i].parent;
                if (tree[p].left == i) {
                    tree[i].parent = -tree[i].parent;
                    tree[i].parent--;
                } else {
                    tree[i].parent++;
                }
            }
            if (size_tree >= 256) {
                fwrite(&tree[i].parent, sizeof(short), 1, OUTPUT_FILE);
            } else {
                char p = tree[i].parent;
                fwrite(&p, sizeof(p), 1, OUTPUT_FILE);
            }
            if (i <= size_forest_copy) {
                short sym = tree[i].symbol;
                sym -= 128;
                char s = sym;
                fwrite(&s, sizeof(char), 1, OUTPUT_FILE);
            }
        }

        while (fscanf(INPUT_FILE, "%c", &c) != -1) {
            for (auto b: encoding[c]) {
                if (written_bits == 8) {
                    written_bits = 0;
                    fwrite(&x, sizeof(char), 1, OUTPUT_FILE);
                    x = 0;
                }
                x <<= 1;
                if (b) {
                    x++;
                }
                written_bits++;
            }
        }
        if (written_bits != 0) {
            while (written_bits != 8) {
                written_bits++;
                x <<= 1;
            }
            fwrite(&x, sizeof(char), 1, OUTPUT_FILE);
        }
    } else {
        FILE *ARCHIVE;
        FILE *OUTPUT_FILE;

        ARCHIVE = fopen(argv[2], "rb");
        OUTPUT_FILE = fopen(argv[3], "wb");

        short tsz, size_forest;
        unsigned char ch;
        int byte_cnt;
        char sfc;
        fread(&tsz, sizeof(short), 1, ARCHIVE);
        sfc = fgetc(ARCHIVE);
        size_forest = sfc + 128;
        if (tsz & (1 << 15)) {
            unsigned short btcnt;
            fread(&btcnt, sizeof(short), 1, ARCHIVE);
            byte_cnt = btcnt;
            tsz ^= (1 << 15);
        } else if (tsz & (1 << 14)) {
            unsigned char k;
            fread(&k, sizeof(unsigned char), 1, ARCHIVE);
            byte_cnt = k;
            tsz ^= (1 << 14);
        } else {
            fread(&byte_cnt, sizeof(int), 1, ARCHIVE);
        }


        Tree t[tsz];
        for (short i = 0; i < tsz; i++) {
            t[i].left = t[i].right = t[i].symbol = -1;
        }

        for (short j = 0; j < tsz; j++) {
            short p;
            char p_char;
            if (tsz >= 256) {
                fread(&p, sizeof(short), 1, ARCHIVE);
            } else {
                fread(&p_char, sizeof(char), 1, ARCHIVE);
                p = p_char;
            }
            if (j <= size_forest) {
                char s = fgetc(ARCHIVE);
                t[j].symbol = s + 128;
            }

            if (p == 0) {
                continue;
            } else if (p < 0) {
                p *= -1;
                p--;
                t[p].left = j;
            } else {
                p--;
                t[p].right = j;
            }
        }

        short v = tsz - 1;
        unsigned char buffer[MAX_BUFFER_SIZE];
        short buffer_size = 0;

        while ((ch = getc(ARCHIVE)) != -1) {
            for (short b = 7; b >= 0; b--) {
                if (ch & (1 << b)) {
                    v = t[v].right;
                } else {
                    v = t[v].left;
                }
                if (t[v].left == -1 && t[v].right == -1) {
                    unsigned char x;
                    x = t[v].symbol;
                    buffer[buffer_size++] = x;
                    if (buffer_size == MAX_BUFFER_SIZE) {
                        for (short j = 0; j < buffer_size; j++) {
                            fwrite(&buffer[j], sizeof(buffer[j]), 1, OUTPUT_FILE);
                        }
                        buffer_size = 0;
                    }
                    v = tsz - 1;
                    byte_cnt--;
                }
                if (!byte_cnt) {
                    for (short j = 0; j < buffer_size; j++) {
                        fwrite(&buffer[j], sizeof(buffer[j]), 1, OUTPUT_FILE);
                    }
                    exit(0);
                }
            }
        }
    }
}
