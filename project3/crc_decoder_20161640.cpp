#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>
#include <math.h>

using namespace std;

int dataword_size;

unsigned char binary_converter(string pass) {
    unsigned char value = 0;
    for(int i = 0; i<(int)pass.length(); i++) {
        value *= 2;
        value += pass[i] - '0';
    }
    return value;
}

string oct_decimal_converter(unsigned char value) {
    string s;
    int div = pow(2, 7);
    for(int i = 0; i<8; i++) {
        if(value / div == 1) {
            s.push_back('1');
            value -= div;
        }
        else s.push_back('0');
        div /= 2;
    }
    return s;
}

char my_xor(char a, char b) {
    if(a==b) return '0';
    else return '1';
}

bool calculate_remain(string data, string generator) {
    string origin = data;
    string remainder;
    int idx = (int)(generator.length() - 1);
    remainder = origin.substr(0, idx);
    for(unsigned int i = 0; i<data.length() - generator.length() + 1; i++) {
        string temp;
        remainder.push_back(origin[idx++]);
        if(remainder[0] == '0') {
            for(unsigned int j = 1; j<generator.length(); j++) temp.push_back(my_xor(remainder[j], '0'));
            remainder = temp;
        }
        else {
            for(unsigned int j = 1; j<generator.length(); j++) temp.push_back(my_xor(remainder[j], generator[j]));
            remainder = temp;
        }
    }

    for(int i = 0; i<(int)remainder.length(); i++) {
        if(remainder[i] != '0') return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    FILE *input_fp, *output_fp, *result_fp;
    string generator, result, ori, ch[3], ans;
    unsigned char zero_padding, st;
    int size, cnt = 0, error_cnt = 0;
    if(argc != 6) {
        fprintf(stderr, "usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        exit(1);
    }

    if((input_fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "input file open error.\n");
        exit(1);
    }

    if((output_fp = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr, "output file open error.\n");
        exit(1);
    }

    if((result_fp = fopen(argv[3], "wb")) == NULL) {
        fprintf(stderr, "result file open error.\n");
        exit(1);
    }

    generator = argv[4];
    dataword_size = atoi(argv[5]);
    if(dataword_size != 4 && dataword_size != 8) {
        fprintf(stderr, "dataword size must be 4 or 8.\n");
        exit(1);    
    }

    int len = dataword_size + (int)(generator.length()) - 1; // 7 , 8
    zero_padding = fgetc(input_fp);
    size = dataword_size == 4? 2*(int)(3+generator.length()) : 7+(int)generator.length(); // 14, 16

    fread(&st, sizeof(unsigned char), 1, input_fp);
    while(feof(input_fp) == 0) {
        result = oct_decimal_converter(st);
        for(int i = 0; i<(int)result.length(); i++) ans.push_back(result[i]);
        fread(&st, sizeof(unsigned char), 1, input_fp);
    }
    for(int j = 0; j < (int)(ans.length() - zero_padding) / size; j++) {
        string s;
        string temp = ans.substr(zero_padding + j*size, size);
        for(int k = 0; k<size/len; k++) {
            string ori = temp.substr(k*len, len);
            cnt++;
            if(!calculate_remain(ori, generator)) error_cnt++;
            s = s + ori.substr(0, dataword_size);
        }
        fputc(binary_converter(s), output_fp);
    }
    fprintf(result_fp, "%d %d", cnt, error_cnt);
    fclose(input_fp);
    fclose(output_fp);
    fclose(result_fp);
    return 0;
}
