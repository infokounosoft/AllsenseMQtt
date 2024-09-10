# 사용법

pub, publocal은 allsense기기와 노트북을 연결하는 port로 전달되는 값을 처리

sub, subwifi는 정해진 주소의 topic을 모두 구독하여 sqlite에 전달 받은 값을 저장한다.

mqtt는 allsense기기 자체에서 wifi접속, ap연결 및 데이터 mqtt 발행을 위해 테스트한 개발 과정에서의 시도들로 최종 적용 본인 iot_edge외의 test 또는 mqtt로 시작하는 파일은 무시해도 상관없다.

# PubLocal
-pub자체에서 sqlite에 저장 mqtt통신을 사용하지 않는다. port로 전달되는 값을 그대로 사용하기 때문

-publocal 폴더를 다운

-npm install 

-npm run build -> 실행 폴더 생성 pub 폴더안에 exebuild 라는 실행 파일 폴더가 생성된다.

-npm run start -> vscode 에서 실행하기 테스트시 npm run start를 사용할 것

-실행 폴더가 생성 되었다면 pub 설치가이드를 따라 진행

# Pub
-port로 전달 받은 값을 0.5s sampling이후 mqtt 통신을 통해 지정된 주소로 발행 topic은 입력 받은 값을 사용

-pub 폴더를 다운

-npm install 

-npm run build -> 실행 폴더 생성 pub 폴더안에 exebuild 라는 실행 파일 폴더가 생성된다.

-npm run start -> vscode 에서 실행하기 테스트시 npm run start를 사용할 것

-실행 폴더가 생성 되었다면 pub 설치가이드를 따라 진행

# Sub
-지정된 주소의 모든 topic을 구독하며 topic에 발행되는 값들을 sqlite에 저장한다.

-sub 폴더, query.txt, 그라파나 설치.txt 를 다운

-npm install 

-npm run build -> 실행 폴더 생성 pub 폴더안에 exebuild 라는 실행 파일 폴더가 생성된다.

-npm run start -> vscode 에서 실행하기 테스트시 npm run start를 사용할 것

-sub 설치가이드를 따라 진행

# SubWiFi
-Allsense 기기 단독으로 wifi에 접속하여 mqtt 발행 시 data 를 모아 보내지 않기 때문에 발행받은 데이터를 받아 0.5초 sampling기능을 추가한 버전.

-기존 pub의 기능일부를 sub에 추가한 버전이다.

-subwifi 폴더, query.txt, 그라파나 설치.txt 를 다운

-npm install 

-npm run build -> 실행 폴더 생성 pub 폴더안에 exebuild 라는 실행 파일 폴더가 생성된다.

-npm run start -> vscode 에서 실행하기 테스트시 npm run start를 사용할 것

-sub 설치가이드를 따라 진행

# exe build 하기

각 폴더들은 electron 파일로써 main, renderer, preload, index.html 파일로 구성되어 있다. 

동봉되어 있는 pakage.json의 npm module 을 설치하기 위해

npm install 을 터미널에 입력해 준다.

exe실행파일을 build하기 위해 npm run build를 터미널에 입력하면 exe파일이 exe result폴더에 생성된다.

파일은 폴더내의 크로니움을 참조하므로 배포시 설치 파일 exe result폴더 전체를 필요로 한다.

# mqtt.ino

mqtt 연결 테스트를 위해 사용한 파일들 이다.

iot_edge_light.cpp의 경우 마지막으로 사용한 최종 버전이다.

test로 시작하는 ino는 개발 과정을 거치며 사용했던 라이브러리 또는 접근 방향으로 최종 버전에 적용되지 않은 것들이다.

차후에 allsense에 업데이트 가능성 有