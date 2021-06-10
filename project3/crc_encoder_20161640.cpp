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

string calculate_encoded_data(string data, string generator) {
    string origin  = data;
    string remainder;
    int idx = (int)(generator.length() - 1);
    for(unsigned int i = 0; i< generator.length()-1; i++) origin.push_back('0');
    remainder = origin.substr(0, idx);
    for(unsigned int i = 0; i<data.length(); i++) {
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
    return data + remainder;
}

int main(int argc, char *argv[]) {

    FILE *input_fp, *output_fp;
    unsigned char store, size;
    string data;
    string generator;
    string result,ans;
    if(argc != 5) {
        fprintf(stderr, "usage: ./crc_encoder input_file output_file generator dataword_size\n");
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

    dataword_size = atoi(argv[4]);
    if(dataword_size != 4 && dataword_size != 8) {
        fprintf(stderr, "dataword size must be 4 or 8.\n");
        exit(1);    
    }
    
    generator = argv[3];
    store = fgetc(input_fp);
    while(feof(input_fp) == 0) {
        data = oct_decimal_converter(store);
        if(dataword_size == 4) {
            string temp1 = calculate_encoded_data(data.substr(0, 4), generator);
            string temp2 = calculate_encoded_data(data.substr(4, 4), generator);
            result = temp1 + temp2;
        }
        else {
            result = calculate_encoded_data(data, generator);
        }
        for(int i = 0; i<(int)result.length(); i++) ans.push_back(result[i]);
        store = fgetc(input_fp);
    }

    ans.length() % 8 != 0 ? size = int(8 - (ans.length()%8)) : 0;
    fwrite(&size, sizeof(unsigned char), 1, output_fp);

    for(int j = 0; j < size; j++) ans = '0' + ans;
    for(int j = 0; j<(int)ans.length()/8; j++) {
        unsigned char temp = binary_converter(ans.substr(8*j, 8));
        fwrite(&temp, sizeof(unsigned char), 1, output_fp);
    }
    fclose(input_fp);
    fclose(output_fp);
    return 0;
}
