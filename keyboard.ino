//74hc165를 사용하기위한 함수.
#include <ShiftIn.h>
//키보드 입력을 사용하기 위한 함수.
#include <Keyboard.h>

//eeprom을 사용(변수값 저장)
#include <EEPROM.h>

//쉬프트 레지스터에서 사용할 특수키.
#define keyCrossbar 16
#define keyTab 17
#define keyBackspace 18

//여기부터 기판의 핀.
#define keyMore 1
#define keyCaps 0

//쉬프트 레지스터(4핀)
#define pLoadPin 2
#define clockEnablePin 3
#define dataPin 4 
#define clockPin 5

//나머지 키.
#define keyShift 6
#define keyCmd 7
#define keyOption 8
#define keySpace 9
#define keyFeed 10
#define keyFn 16

//74hc165갯수.
ShiftIn<6> shift;

//현재 접촉된 키를 구한다
int key=-1;
//마지막으로 누른 키를 구한다
int lastKey=-1;
//키가 눌리고 있으면 true
bool isPressing=false;
//눌리고 있는 키의 갯수
int numKey=0;

//입력이 감지될 경우 이 배열에서 대응하는 문자를 입력한다.
char keySettings[48]={'0','l','o',',','8','j','u','n','=','\'','\\','/','9','k','i','m','\0','\0','\0','\0','-',';','p','.','7','h','y','b','5','f','r','c','6','g','t','v','3','s','w','z','4','d','e','x','2','a','q','1'};
//fn 키를 누를 때.
char keyAddon[48]={KEY_F10,KEY_PAGE_UP,'[',KEY_END,KEY_F8,'/','u','+',KEY_F12,KEY_RIGHT_ARROW,KEY_UP_ARROW,KEY_DOWN_ARROW,KEY_F9,KEY_HOME,KEY_INSERT,'-','\0','\0','\0','\0',KEY_F11,KEY_LEFT_ARROW,']',KEY_PAGE_DOWN,KEY_F7,'*','y','b',KEY_F5,'f','r','c',KEY_F6,'g','t','v',KEY_F3,'s','w','z',KEY_F4,'d','e','x',KEY_F2,'a',KEY_ESC,KEY_F1};

//fn키가 눌려있는가?
bool isFn=false;
//auto키가 눌려있는가?
bool isFeed=false;
//caps키가 눌려있는가?
bool isCaps=false;

//윈도우 모드인가?
bool winMode=false;

//기초 설정.
void setup() {
  //디버그용 시리얼통신(9600bps)
  //Serial.begin(9600);
  // 74hc165 핀 설정: pLoadPin, clockEnablePin, dataPin, clockPin
  shift.begin(pLoadPin, clockEnablePin, dataPin, clockPin);
  //캡스락 버튼
  pinMode(keyCaps,INPUT);
  //더찍기 버튼(tab키로 할당)
  pinMode(keyMore,INPUT);
  
  //쉬프트 버튼.
  pinMode(keyShift,INPUT);
  //커맨드 버튼.
  pinMode(keyCmd,INPUT);
  //옵션 버튼.
  pinMode(keyOption,INPUT);
  //스페이스바.
  pinMode(keySpace,INPUT);
  //feed버튼(윈도우에서는 한영키)
  pinMode(keyFeed,INPUT);
  //fn버튼.
  pinMode(keyFn,INPUT);
  
  //키보드 시작
  Keyboard.begin();

  //EEPROM.write(0, true);
  if(EEPROM.read(0)==true){
    winMode=true;
  }
}
//74hc165 입력 처리
void displayValues() {
  numKey=0;
  //tab키가 눌리면 반응을 해준다 
  if(shift.state(keyTab) == false){
    Keyboard.press(KEY_RETURN);
  }
  else{
    Keyboard.release(KEY_RETURN);
  }

  //backspace키가 눌리면 반응을 해준다 
  if(shift.state(keyBackspace)){
    if(isFn){
      Keyboard.press(KEY_DELETE);
    }
    else{
      Keyboard.press(KEY_BACKSPACE);
    }
  }
  else{
    if(isFn){
      Keyboard.release(KEY_DELETE);
  }
    else{
      Keyboard.release(KEY_BACKSPACE);
    }
  }
  
  //크로스바가 들어질 때부터 인식을 시작한다.
  if(shift.state(keyCrossbar)){
    
    //쉬프트 레지스터 상태 확인.
    for(int i = 0; i < shift.getDataWidth(); i++){
      //디버그용 출력.
      //Serial.print( shift.state(i) );
      
      //크로스바와 탭 키는 동시입력제한에서 제외시킨다.
      if(i!=keyCrossbar 
      && i!=keyTab
      && shift.state(i)==1){
        key=i;
        numKey++;
        }
    }
    //동시입력으로 오류가 생길경우 무시한다.
    if(numKey>1){
      key=-1;
      //Serial.print(numKey);
      //Serial.println();
    }
    
    if(key!=-1){
      isPressing=true;
      //빨리 쳐서 크로스바가 내려가기 전에 쳤을 경우
      if(lastKey!=-1 && lastKey!=key){
        if(isFn){
          Keyboard.release(keyAddon[lastKey]);
        }
        else{
          Keyboard.release(keySettings[lastKey]);
        }
      }
      lastKey=key;
    }
  }
  //크로스바가 떨어졌을경우 
  else{
    //인식을 해제시켜야 하므로 -1로 지정한다.
    key=-1;
    //키가 떨어질 때만 false
    if(shift.state(keyCrossbar)==false){
      if(isPressing==true){
        isPressing=false;
        //Serial.print("keyUp");
        //Serial.print(lastKey);
        //Serial.println();
        if(isFn){
          Keyboard.release(keyAddon[lastKey]);
        }
        else{
          Keyboard.release(keySettings[lastKey]);
        }
        lastKey=-1;
      }
      //오류를 대비해서 기본상태는 false
      isPressing=false;
      }
    
  }
  //아무 키라도 인식됬을경우 
  if(key!=-1){
    //Serial.print(" , ");
    //Serial.print(key);
    //Serial.println();
    if(isFn){
      Keyboard.press(keyAddon[key]);
    }
    else{
      Keyboard.press(keySettings[key]);
    }
  }
}

void loop() {
  //변동사항이 있을 시 함수 호출 
  if(shift.update())
    displayValues();

  //캡스락 키를 눌렀을경우.
  if(digitalRead(keyCaps)){
    if(isCaps==false){
      if(isFn){
        Keyboard.press(KEY_LEFT_CTRL);
      }
      else{
        Keyboard.press(KEY_CAPS_LOCK);
      }
    }
    isCaps=true;
  }
  else{
    if(isCaps==true){
      if(isFn){
        Keyboard.release(KEY_LEFT_CTRL);
      }
      else{
        Keyboard.release(KEY_CAPS_LOCK);
      }
    }
    isCaps=false;
  }

  //탭 키를 눌렀을경우.
  if(digitalRead(keyMore)){
    if(isFn){
      Keyboard.press('`');
    }
    else{
      Keyboard.press(KEY_TAB);
    }
  }
  else{
    if(isFn){
      Keyboard.release('`');
    }
    else{
      Keyboard.release(KEY_TAB);
    }
  }

  //shift 키를 눌렀을경우.
  if(digitalRead(keyShift) == false){
    //Serial.println("shift");
    Keyboard.press(KEY_LEFT_SHIFT);
  }
  else{
    Keyboard.release(KEY_LEFT_SHIFT);
  }

  //cmd 키를 눌렀을경우.
  if(digitalRead(keyCmd)){
    //Serial.println("cmd");
    if(winMode == true)
      Keyboard.press(KEY_LEFT_ALT);
    else
      Keyboard.press(KEY_LEFT_GUI);
  }
  else{
    if(winMode == true)
      Keyboard.release(KEY_LEFT_ALT);
    else
      Keyboard.release(KEY_LEFT_GUI);
  }

  //option 키를 눌렀을경우.
  if(digitalRead(keyOption)){
    //Serial.println("option");
    if(winMode == true)
      Keyboard.press(KEY_LEFT_CTRL);
    else
      Keyboard.press(KEY_LEFT_ALT);
  }
  else{
    if(winMode == true)
      Keyboard.release(KEY_LEFT_CTRL);
    else
      Keyboard.release(KEY_LEFT_ALT);
  }

  //space 키를 눌렀을경우.
  if(digitalRead(keySpace)==false){
    //Serial.println("Space");
    Keyboard.press(' ');
  }
  else{
    Keyboard.release(' ');
  }

  //feed 키를 눌렀을경우.
  if(digitalRead(keyFeed)==false){
    //Serial.println("feed");
    if(isFeed==false)
    {
      //fn+auto키면 모드 변경.
      if(isFn==true){
        Keyboard.releaseAll();
        winMode= !winMode;
        //값 저장.
        EEPROM.write(0, winMode);
        }
      else{
        if(winMode==true)
          Keyboard.press(KEY_RIGHT_ALT);
        else
          Keyboard.press(KEY_CAPS_LOCK);
        }
    }
    isFeed=true;
  }
  else{
    if(isFeed==true)
    {
       if(winMode==true)
          Keyboard.release(KEY_RIGHT_ALT);
       else
          Keyboard.release(KEY_CAPS_LOCK);
    }
    isFeed=false;
  }

  //fn 키를 눌렀을경우.
  if(digitalRead(keyFn)){
    //눌리는 첫 이벤트만 감지해야 하므로.
    if(isFn==false)
    {
      Keyboard.releaseAll();
    }
    isFn=true;
  }
  else{
    if(isFn==true)
    {
      Keyboard.releaseAll();
    }
    isFn=false;
  }
  delay(1);
}
