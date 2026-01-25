# Contributing to Rhee Creatives Linux
# Rhee Creatives Linux 기여하기

We welcome contributions to **Rhee Creatives Linux v1.0 - Extreme Performance Edition**!
**Rhee Creatives Linux v1.0 - 익스트림 퍼포먼스 에디션**에 대한 기여를 환영합니다!

Please follow these guidelines to ensure a smooth collaboration process.
원활한 협업 과정을 위해 다음 가이드라인을 따라 주십시오.

## 🤝 Code of Conduct / 행동 강령

- **Respect**: Treat all contributors with respect.
  - **존중**: 모든 기여자를 존중해 주세요.
- **Bilingual**: All code comments and documentation MUST be bilingual (Korean First, English Second).
  - **이중 언어**: 모든 코드 주석과 문서는 반드시 이중 언어(한국어 우선, 영어 차선)로 작성되어야 합니다.
- **Optimization**: Code should focus on extreme performance and "Spider-Web" robustness.
  - **최적화**: 코드는 극한의 성능과 "거미줄" 같은 견고함에 중점을 두어야 합니다.

## 🛠️ How to Contribute / 기여 방법

1.  **Fork the Repository** / **저장소 포크하기**
    - Click the 'Fork' button on GitHub.
    - GitHub에서 'Fork' 버튼을 클릭하세요.

2.  **Create a Branch** / **브랜치 생성하기**
    ```bash
    git checkout -b feature/AmazingFeature
    ```

3.  **Make Changes** / **변경 사항 적용하기**
    - Follow the bilingual comment style:
    - 이중 언어 주석 스타일을 따르세요:
      ```c
      /*
       * Check system integrity.
       * 시스템 무결성을 확인합니다.
       */
      ```

4.  **Test Your Changes** / **변경 사항 테스트하기**
    - Build and run using Docker:
    - Docker를 사용하여 빌드 및 실행하세요:
      ```bash
      ./run_linux.sh
      ```

5.  **Commit and Push** / **커밋 및 푸시하기**
    ```bash
    git commit -m "FEAT: Add amazing feature / 놀라운 기능 추가"
    git push origin feature/AmazingFeature
    ```

6.  **Open a Pull Request** / **풀 리퀘스트 열기**
    - Describe your changes in both Korean and English.
    - 변경 전후 스크린샷이 있다면 첨부해 주세요.

## 📝 License / 라이선스

By contributing, you agree that your contributions will be licensed under the **Apache License 2.0**.
기여함으로서, 귀하의 기여가 **Apache License 2.0** 하에 라이선스됨에 동의하는 것으로 간주됩니다.
