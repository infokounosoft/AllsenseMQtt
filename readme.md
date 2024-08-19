# 사용법

# Pub
-pub 폴더를 다운

-pub 설치가이드를 따라 진행

# Sub
-sub 폴더, query.txt, 그라파나 설치.txt 를 다운

-sub 설치가이드를 따라 진행

# exe build 하기

각 폴더들은 electron 파일로써 main, renderer, preload, index.html 파일로 구성되어 있다. 

동봉되어 있는 pakage.json의 npm module 을 설치하기 위해

npm install 을 터미널에 입력해 준다.

exe실행파일을 build하기 위해 npm run build를 터미널에 입력하면 exe파일이 exe result폴더에 생성된다.

파일은 폴더내의 크로니움을 참조하므로 배포시 설치 파일 전체를 필요로 한다.

# mqtt.ino

사용하는 내 문서 -> arduino -> library 폴더의 mics6814-i2c 폴더를

mqtt 폴더안의 mics6814-i2c로 교체