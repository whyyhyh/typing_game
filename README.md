# typing_game
system programming

# 타이핑 연습 게임 (typing\_game)

간단한 터미널 기반 타이핑 연습 프로그램입니다. 부모(게임 로직)와 자식(타이머) 프로세스로 구성되며, 쉘스크립트(Makefile, run\_typing\_game.sh)로 자동 빌드·실행됩니다.

1. 주요 기능

---

* **모드 선택**: 알파벳, 단어, 문장 연습(30초 제한)
* **점수 집계**: 정확히 입력 시 1점 추가
* **카운트다운 타이머**: 자식 프로세스에서 `SIGALRM`, `itimerval` 사용
* **종료 신호**: 타임아웃 시 자식 → 부모 `SIGUSR1` 전송
* **IPC**: `pipe()` + 논블록킹 `read()`/`write()`로 남은 시간 전달
* **키보드 제어**: `termios` 로 논캐논·비에코 모드, `select()`로 입력 감지
* **ANSI 제어**: 화면 깜빡임 최소화, 제시어·상태 덮어쓰기
* **쉘스크립트**:

  * `Makefile` (`make`, `make clean`, `make typing_game`)
  * `run_typing_game.sh` (단일 스크립트로 빌드 및 실행 자동화)

2. 요구 사항

---

* Unix 계열 OS (Linux, macOS)
* GCC 또는 호환 C 컴파일러
* 터미널(ANSI 이스케이프 지원)

3. 파일 구조

---

```
programgyuri/
├── typing_game.c          # 게임 소스 코드
├── Makefile               # 빌드 및 실행 (make)
├── run_typing_game.sh     # 쉘스크립트 (컴파일 및 실행)
└── README.txt             # 설명 문서 (이 파일)
```

4. 빌드 & 실행

---

```bash
# Makefile 사용
make clean               # 이전 빌드 파일 제거
make typing_game         # 컴파일 후 실행

# 또는 쉘스크립트 사용
chmod +x run_typing_game.sh
./run_typing_game.sh     # 빌드 & 실행
```

5. 사용 방법

---

1. 메뉴에서 연습 모드를 선택

2. 첫째 줄에 제시어(알파벳·단어·문장) 출력

3. 둘째 줄에서 `시간·점수·입력:` 프롬프트에 입력 후 **Enter**

   * **스페이스**: 띄어쓰기 허용
   * **백스페이스**: 오타 수정

4. 제출 시 새로운 제시어 첫째 줄 덮어쓰기

5. 30초 후 자동 종료 및 최종 점수 출력

6. 종료 후 메뉴로 복귀 또는 `4. 종료` 선택

7. 설정

---

* `TOTAL_TIME` (30초)
* `MAX_INPUT_LEN` (입력 최대 길이)
* `PROMPT_WIDTH` (제시어 폭)
* 제시어 리스트: `alphabet_list`, `word_list`, `sentence_list`

7. 라이선스

---

MIT License

© 2025 Gyuri Potter. All rights reserved.

