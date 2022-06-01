//tipssoft 소스코드 인용
//stackoverflow의 도움을 받음
//MSDN 문서를 참조함

// 소리의 불명확성을 이용하여 자연난수를 구현.
// 제작 : 30302 김규석


#define _CRT_SECURE_NO_WARNINGS //보안 경고 해제
#define _CRT_OBSOLETE_NO_WARNINGS 
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <openssl/sha.h> //Sha 해시 함수 헤더 (Precompiled Openssl 라이브러리)
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <WELL1024a.h> //Well Random Algorithm 헤더
using namespace std;

//소리 추출 부
#define WAVE_FORMAT_PCM 1
#define MONO 1
#define STEREO 2
#define SAMPLE_RATE 44100 //44100 or 22050 -> 44100은 음질 저하됨.
#define BIT_PER_SAMPLE 16
#define BIT 8

#define BUFFER_COUNT 2
#define BUFFER_SIZE 1024 * 2

//Well1024a 부
#define W 32
#define R 32
#define M1 3
#define M2 24
#define M3 10
#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define Identity(v) (v)
#define V0            STATE[state_i                   ]
#define VM1           STATE[(state_i+M1) & 0x0000001fU]
#define VM2           STATE[(state_i+M2) & 0x0000001fU]
#define VM3           STATE[(state_i+M3) & 0x0000001fU]
#define VRm1          STATE[(state_i+31) & 0x0000001fU]
#define newV0         STATE[(state_i+31) & 0x0000001fU]
#define newV1         STATE[state_i                   ]
#define FACT 2.32830643653869628906e-10

typedef struct {
    char szChunkID[4];
    unsigned int nChunkSize;
    char szFormat[4];
    char szSubChunk1ID[4];
    unsigned int nSubChunk1Size;
    unsigned short sAudioFormat;
    unsigned short sNumOfChannels;
    unsigned int nSampleRate;
    unsigned int nByteRate;
    unsigned short sBlockAlign;
    unsigned short sBitsPerSample;
    char szSubChunk2ID[4];
    unsigned int nSubChunk2Size;
}WAV_FORMAT;

WAV_FORMAT header;
WAVEHDR* wave_data;
FILE* fp;

void initHeader();
void recordVoice(WAVEHDR*);

static unsigned int state_i = 0;
static unsigned int STATE[R];
static unsigned int z0, z1, z2;

void InitWELLRNG1024a(unsigned int* init) {
    int j;
    state_i = 0;
    for (j = 0; j < R; j++)
        STATE[j] = init[j];
}

double WELLRNG1024a(void) {
    z0 = VRm1;
    z1 = Identity(V0) ^ MAT0POS(8, VM1);
    z2 = MAT0NEG(-19, VM2) ^ MAT0NEG(-14, VM3);
    newV1 = z1 ^ z2;
    newV0 = MAT0NEG(-11, z0) ^ MAT0NEG(-7, z1) ^ MAT0NEG(-13, z2);
    state_i = (state_i + 31) & 0x0000001fU;
    return ((double)STATE[state_i] * FACT);
}

void CALLBACK stream_waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    WAVEHDR* pHdr = NULL;
    switch (uMsg)
    {
    case WIM_CLOSE:
        break;

    case WIM_DATA://데이터
        pHdr = (WAVEHDR*)dwParam1;
        recordVoice(pHdr);
        if (waveInAddBuffer(hwi, pHdr->lpNext, sizeof(WAVEHDR))) {
            printf("버퍼 오류. 프로그램을 재실행하여 주십시오.\n");
            exit(1);
        }
        break;
    case WIM_OPEN:
        break;
    default:
        break;
    }
}

void sha256_hash_string(unsigned char hash[SHA256_DIGEST_LENGTH], char* outputBuffer)
{
    int i = 0;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

int calc_sha256(const char* path, char* output) {
    //파일을 엽니다.
    FILE* file = fopen(path, "rb");
    if (!file)return 1;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    const int bufSize = 32768;
    unsigned char* buffer = (unsigned char*)malloc(bufSize);
    SHA256_CTX sha256;
    int bytesRead = 0;

    if (!buffer)return 1;

    SHA256_Init(&sha256);

    while ((bytesRead = fread(buffer, 1, bufSize, file))) {
        SHA256_Update(&sha256, buffer, bytesRead);
    }

    SHA256_Final(hash, &sha256);
    sha256_hash_string(hash, output);
    fclose(file);

    return 0;
}

int main(int argc, char * argv[]) {
    unsigned int record_t = 300;
    unsigned int random_range = 0;
    int chk_boolean;
    if (argv[1] != NULL) {
        chk_boolean = (strcmp(argv[1], "-h") == 0);
        if (chk_boolean) {
            printf("%s [-h -t] (integer 1~3000) (integer 1~2147483646)\n\n[-h -t]\n-h : 도움말을 엽니다\n-t : 프로그램 실행을 시작합니다\n\n(integer 1~3000) : 소리 추출 시간을 지정합니다. 정수 1은 10ms 이며, 최소 값은 50, 최대 값은 3000입니다.\n500ms 이하의 시간은 추출이 원활하지 않아 무시되며, 3초 이상을 권장합니다.\n\n(integer 1~2147483646) : 난수 범위를 지정합니다. 최소 값은 1, 최대 값은 uint32_max - 1 입니다.\n\n(integer ) 인수에는 지정된 범위의 정수만 입력하십시오. 예상치 못한 동작을 할 수 있습니다.\n모든 프로그램 처리에 있어 오류가 발생하면 1을 반환합니다.\n\n", argv[0]);
            return 0;
        }
    }
    if (argv[1] != NULL && argv[2] != NULL && argv[3] != NULL) {
        chk_boolean = (strcmp(argv[1], "-t") == 0);
        if (!chk_boolean) {
            printf("잘못된 인수 : %s", argv[1]);
            return 1;
        }
        record_t = atoi(argv[2]);
        random_range = atoi(argv[3]);
        if (record_t < 1 || record_t > 3000) {
            printf("입력된 추출 시간이 지정 범위를 초과했거나 도달하지 못했습니다 : %d", record_t);
            return 1;
        }
        else if (random_range < 1 || random_range > 2147483646) {
            printf("입력된 난수 범위가 지정 범위를 초과했거나 도달하지 못했습니다 : %d", random_range);
            return 1;
        }
        printf("시간 : %d 범위 : %d\n", record_t, random_range);
    }
    else {
        printf("[소리를 이용한 난수 구현] 제작 : 30302 김규석\n\nUsage : \"프로그램 경로\" [-h -t] (integer 1~3000) (integer 1~2147483646)\n\n자세한 사용법은 \"%s\" -h (을)를 사용하십시오\n\n", argv[0]);
        return 0;
    }
    fp = fopen("data.wav", "wb");
    initHeader();
    fwrite(&header, sizeof(header), 1, fp);

    WAVEFORMATEX stream_InFormat; //음성에 대한 초기값들에 대한 구조체
    WAVEHDR stream_WaveInHdr[BUFFER_COUNT]; //입력// 메모리공간과 버퍼에 대한 연결을 담당하는 구조체
    WAVEHDR stream_WaveOutHdr[BUFFER_COUNT]; //출력// 메모리공간과 버퍼에 대한 연결을 담당하는 구조체
    HWAVEIN stream_hWaveIn; // 입력 핸들러
    HWAVEOUT stream_hWaveOut; //출력 핸들러

    unsigned char* stream_pWaveInBuffer[BUFFER_COUNT]; // 입력버퍼

    //초기값 선언
    stream_InFormat.wFormatTag = WAVE_FORMAT_PCM;
    stream_InFormat.wBitsPerSample = BIT_PER_SAMPLE;
    stream_InFormat.nChannels = MONO;
    stream_InFormat.nSamplesPerSec = SAMPLE_RATE;
    stream_InFormat.nBlockAlign = MONO * BIT_PER_SAMPLE / BIT;
    stream_InFormat.nAvgBytesPerSec = stream_InFormat.nBlockAlign * SAMPLE_RATE;
    stream_InFormat.cbSize = 0;

    //버퍼 메모리 할당
    for (int i = 0; i < BUFFER_COUNT; i++) {
        stream_pWaveInBuffer[i] = (unsigned char*)malloc(BUFFER_SIZE * stream_InFormat.nBlockAlign);
        ZeroMemory(stream_pWaveInBuffer[i], BUFFER_SIZE * stream_InFormat.nBlockAlign);
    }

    //입력할 버퍼를 지정
    for (int i = 0; i < BUFFER_COUNT; i++) {
        stream_WaveInHdr[i].lpData = (LPSTR)stream_pWaveInBuffer[i]; //현재 버퍼
        stream_WaveInHdr[i].dwBufferLength = BUFFER_SIZE * stream_InFormat.nBlockAlign;
        stream_WaveInHdr[i].dwBytesRecorded = 0;
        stream_WaveInHdr[i].dwUser = 0;
        stream_WaveInHdr[i].dwFlags = 0;
        stream_WaveInHdr[i].dwLoops = 0;
        stream_WaveInHdr[i].lpNext = &stream_WaveInHdr[(i + 1) % BUFFER_COUNT];  //다음 버퍼의 위치
    }

    //CALLBACK 함수 등록과 핸들러 지정
    if (waveInOpen(&stream_hWaveIn, WAVE_MAPPER, &stream_InFormat, (DWORD_PTR)stream_waveInProc, NULL, CALLBACK_FUNCTION))
    {
        printf("waveform input device(을)를 열 수 없습니다.\n");
        exit(0);
    }

    //버퍼를 등록
    for (int i = 0; i < BUFFER_COUNT; i++) {
        if (waveInPrepareHeader(stream_hWaveIn, &stream_WaveInHdr[i], sizeof(WAVEHDR))) {
            MessageBoxA(NULL, "prepareHeader를 실패했습니다.", NULL, MB_OK | MB_ICONEXCLAMATION);
            exit(0);
        }
    }
    //버퍼를 대기
    if (waveInAddBuffer(stream_hWaveIn, &stream_WaveInHdr[0], sizeof(WAVEHDR)))
    {
        MessageBoxA(NULL, "장치로부터 읽기 엑세스가 금지되었습니다.", NULL, MB_OK | MB_ICONEXCLAMATION);
        exit(0);
    }

    //음성 입력 시작
    if (waveInStart(stream_hWaveIn))
    {
        MessageBoxA(NULL, "음성 데이터 녹음을 시작할 수 없습니다.", NULL, MB_OK | MB_ICONEXCLAMATION);
        exit(0);
    }
    printf("소리데이터 추출 중 입니다. 정의된 추출 시간 : %.1fs\n", (float)record_t / 100.0);
    while (true)
    {
        _sleep(record_t * 10);
        break;
    }
    //메모리 해제
    for (int i = 0; i < BUFFER_COUNT; i++) {
        waveInUnprepareHeader(stream_hWaveIn, &stream_WaveInHdr[i], sizeof(WAVEHDR));
    }
    //종료
    waveInClose(stream_hWaveIn);
    //메모리 해제
    //메모리 반환
    for (int i = 0; i < BUFFER_COUNT; i++) {
        free(stream_pWaveInBuffer[i]);
    }
    fclose(fp);

    char calc_hash[65];
    unsigned int calc_hash_int[64] = {0, };
    unsigned char sha_digest[SHA256_DIGEST_LENGTH];

    const char path_s[] = "data.wav";
    int result;
    result = calc_sha256(path_s, calc_hash);
    if (result != 0) {
        printf("오류가 발생했습니다. calc_sha256()에서 잘못된 값을 반환했습니다.");
        return 1;
    }
    printf("%s\n", calc_hash);
    for (unsigned int i = 0; i < strlen(calc_hash); i++)
    {
        calc_hash_int[i] = calc_hash[i] - '0';
    }
    srand((unsigned)time(NULL));
    unsigned int init[32];
    for (int i = 0; i < 32; i++) {
        init[i] = rand() << 16 | calc_hash_int[i];
    }
    InitWELLRNG1024a(init);
    printf("결과 : %d", (int)((double)WELLRNG1024a() * (random_range + 1)));
    result = remove(path_s);
    if (result)
        printf("소리 데이터 삭제를 실패했습니다.");
        return 1;
}

void recordVoice(WAVEHDR* wave) {
    //헤더에 파일의 길이값을 수정해줍니다. 
    //언제 끝날지 모르기때문에 항상 해줍니다.
    header.nChunkSize += wave->dwBufferLength;
    header.nSubChunk2Size += wave->dwBufferLength;
    fseek(fp, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, fp);

    //뒤에는 음성값을 파일에 씁니다.
    fseek(fp, 0, SEEK_END);
    fwrite(wave->lpData, wave->dwBufferLength, 1, fp);
}

void initHeader() {
    memcpy(&header.szChunkID, "RIFF", 4);
    header.nChunkSize = 36;//file총 길이 - 8;
    memcpy(&header.szFormat, "WAVE", 4);
    memcpy(&header.szSubChunk1ID, "fmt ", 4);

    header.nSubChunk1Size = 16;
    header.sAudioFormat = WAVE_FORMAT_PCM;
    header.sNumOfChannels = MONO;
    header.nSampleRate = SAMPLE_RATE;
    header.nByteRate = SAMPLE_RATE * 2;
    header.sBlockAlign = MONO * BIT_PER_SAMPLE / BIT;
    header.sBitsPerSample = BIT_PER_SAMPLE;
    memcpy(&header.szSubChunk2ID, "data", 4);
    header.nSubChunk2Size = 0; //data의길이 
}