#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/wait.h>

#define TOTAL_TIME      30    // 제한 시간(초)
#define MAX_INPUT_LEN   64    // 입력 최대 길이
#define PROMPT_WIDTH    40    // 제시어 고정 폭

volatile sig_atomic_t time_up = 0;

void sigalrm_handler(int sig) { }
void sigusr1_handler(int sig) { time_up = 1; }

void timer_process(int write_fd) {
    struct sigaction sa = {0};
    sa.sa_handler = sigalrm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    struct itimerval tv = {{1,0},{1,0}};
    setitimer(ITIMER_REAL, &tv, NULL);

    int remaining = TOTAL_TIME;
    char buf[16];
    while (remaining > 0) {
        pause();
        remaining--;
        int n = snprintf(buf, sizeof(buf), "%d", remaining);
        write(write_fd, buf, n+1);
    }
    kill(getppid(), SIGUSR1);
    close(write_fd);
    exit(0);
}

int main() {
    const char *alphabet_list[] = {"A","B","C","D","E","F","G","H","I","J",
        "K","L","M","N","O","P","Q","R","S","T",
        "U","V","W","X","Y","Z"};
    int alphabet_count = sizeof(alphabet_list)/sizeof(alphabet_list[0]);

    const char *word_list[] = {"hello","world","typing","signal","countdown","game",
        "apple","banana","orange","river","spirit","garden","melody","sunset","whisper","courage","meadow","compass","horizon","blossom","velvet","spark"};
    int word_count = sizeof(word_list)/sizeof(word_list[0]);

    const char *sentence_list[] = {
        "Practice makes perfect",
        "The quick brown fox jumps over the lazy dog",
        "Typing games are fun",
        "Life is a journey, not a destination",
        "Dreams don’t work unless you do",
        "Actions speak louder than words",
        "To be or not to be, that is the question",
        "Every cloud has a silver lining",
        "The early bird catches the worm",
        "Knowledge is power; use it wisely",
        "Time waits for no one, make it count",
        "Happiness is found within yourself",
        "The best way out is always through"
    };
    int sentence_count = sizeof(sentence_list)/sizeof(sentence_list[0]);

    while (1) {
        printf("\n=== 타이핑 연습 메뉴 ===\n");
        printf("1. 알파벳 연습\n");
        printf("2. 단어 연습\n");
        printf("3. 문장 연습\n");
        printf("4. 종료\n");
        printf("선택 (1-4)> ");

        int mode = 0;
        if (scanf("%d", &mode) != 1) {
            while (getchar()!='\n');
            continue;
        }
        if (mode == 4) {
            printf("프로그램을 종료합니다.\n");
            break;
        }
        if (mode < 1 || mode > 3) {
            printf("올바른 번호를 입력하세요.\n");
            continue;
        }
        getchar();

        const char **items;
        int item_count;
        if (mode == 1) {
            items = alphabet_list; item_count = alphabet_count;
        } else if (mode == 2) {
            items = word_list; item_count = word_count;
        } else {
            items = sentence_list; item_count = sentence_count;
        }

        int pipefd[2];
        if (pipe(pipefd) < 0) { perror("pipe"); continue; }
        signal(SIGUSR1, sigusr1_handler);
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); close(pipefd[0]); close(pipefd[1]); continue; }

        if (pid == 0) {
            close(pipefd[0]);
            timer_process(pipefd[1]);
        } else {
            close(pipefd[1]);
            fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

            struct termios orig, raw;
            tcgetattr(STDIN_FILENO, &orig);
            raw = orig;
            raw.c_lflag &= ~(ICANON|ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);

            srand(time(NULL));
            int remaining = TOTAL_TIME;
            int score = 0;
            int pos = 0;
            char input[MAX_INPUT_LEN] = {0};
            int idx = rand() % item_count;

            // 첫 줄: 제시어 출력
            printf("제시어: %s\n", items[idx]);
            // 둘째 줄: 시간·점수·입력
            printf("시간: %2d초  점수: %3d  입력: ", remaining, score);
            fflush(stdout);

            fd_set rfds;
            while (!time_up) {
                // 둘째 줄만 덮어쓰기 (시간·점수·입력)
                printf("\r\033[K시간: %2d초  점수: %3d  입력: %-*s",
                       remaining, score, MAX_INPUT_LEN-1, input);
                fflush(stdout);

                FD_ZERO(&rfds);
                FD_SET(STDIN_FILENO, &rfds);
                FD_SET(pipefd[0], &rfds);
                int nf = (STDIN_FILENO>pipefd[0]?STDIN_FILENO:pipefd[0]) + 1;
                if (select(nf, &rfds, NULL, NULL, NULL) < 0) continue;

                if (FD_ISSET(pipefd[0], &rfds)) {
                    char buf[16]; int n = read(pipefd[0], buf, sizeof(buf));
                    if (n > 0) remaining = atoi(buf);
                }
                if (FD_ISSET(STDIN_FILENO, &rfds)) {
                    char c;
                    if (read(STDIN_FILENO, &c, 1) > 0) {
                        if (c == '\n') {
                            input[pos] = '\0';
                            if (strcmp(input, items[idx]) == 0) score++;
                            pos = 0; memset(input, 0, sizeof(input));
                            idx = rand() % item_count;
                            // 커서를 첫 줄(제시어)로 이동 후 덮어쓰기
                            printf("\033[1F\r\033[K제시어: %s\n", items[idx]);
                            // 둘째 줄 초기화
                            printf("시간: %2d초  점수: %3d  입력: ", remaining, score);
                            fflush(stdout);
                        } else if (c == 127 || c == 8) {
                            if (pos > 0) input[--pos] = '\0';
                        } else if (isprint(c) && pos < MAX_INPUT_LEN - 1) {
                            input[pos++] = c;
                            input[pos] = '\0';
                        }
                    }
                }
            }
            tcsetattr(STDIN_FILENO, TCSANOW, &orig);
            printf("\n\n⏰ 시간 종료! 최종 점수: %d점\n", score);
            wait(NULL);
            time_up = 0;
        }
    }
    return 0;
}
