#include <stdint.h>

static const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline size_t ustrlen(const char* str) {
	size_t length = 0;
	while (*str != '\0') {
		++length;
		++str;
	}
	return length;
}
inline char* Base64Encoder(const char* input)
{
	uint32_t charlen = ustrlen(input); // �Է� ���ڿ��� ���� ��������
	uint32_t outputSize = ((charlen + 2) / 3) * 4 + 1; // Base64 �������� ��ȯ�� ���ڿ��� ���� ���� ���
	char* output = new char[outputSize]; // ���� ���̿� �°� �޸� �Ҵ�
	uint32_t length = 0;

	for (uint32_t i = 0; i < charlen; i += 3) {
		uint32_t triple = 0;

		for (uint32_t j = 0; j < 3; ++j) {
			triple <<= 8;
			if (i + j < charlen) {
				triple |= (uint8_t)input[i + j];
			}
		}

		for (int j = 0; j < 4; ++j) {
			uint32_t index = (triple >> (6 * (3 - j))) & 0x3F;
			output[length++] = base64Chars[index];
		}
	}

	while (length % 4 != 0) {
		output[length++] = '=';
	}

	output[length] = '\0';
	return output;
}
