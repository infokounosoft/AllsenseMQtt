import sys 
from PyQt5.QtWidgets import *
from PyQt5 import uic
from test import download
formMain = uic.loadUiType("qt.ui")[0]

class MyWindow(QMainWindow,formMain): #윈도우,ui모두 상속
    fileName = ""
    topic = ""
    status = 2
    a = ""
    cb1 = 0
    cb2 = 0
    cb3 = 0
    cb4 = 0
    cb5 = 0
    cb6 = 0

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.btn1.clicked.connect(self.btn1Clicked)
        self.led1.textChanged.connect(self.led1Changed)
        self.led2.textChanged.connect(self.led2Changed)
        self.rbn1.toggled.connect(self.rbn1Checked)
        self.rbn2.toggled.connect(self.rbn2Checked)
        self.rbn3.toggled.connect(self.rbn3Checked)
        self.rbn1.setChecked(True)
        self.cb1.stateChanged.connect(self.cb1Checked)
        self.cb2.stateChanged.connect(self.cb2Checked)
        self.cb3.stateChanged.connect(self.cb3Checked)
        self.cb4.stateChanged.connect(self.cb4Checked)
        self.cb5.stateChanged.connect(self.cb5Checked)
        self.cb6.stateChanged.connect(self.cb6Checked)
        self.cb1 = 0
        self.cb2 = 0
        self.cb3 = 0
        self.cb4 = 0
        self.cb5 = 0
        self.cb6 = 0
        
    
    def cb1Checked(self):
        if self.cb1 == 0:
            self.cb1 = 1
        else:
            self.cb1 = 0

    def cb2Checked(self):
        if self.cb2 == 0:
            self.cb2 = 1
        else:
            self.cb2 = 0
            
    def cb3Checked(self):
        if self.cb3 == 0:
            self.cb3 = 1
        else:
            self.cb3 = 0
            
    def cb4Checked(self):
        if self.cb4 == 0:
            self.cb4 = 1
        else:
            self.cb4 = 0
            
    def cb5Checked(self):
        if self.cb5 == 0:
            self.cb5 = 1
        else:
            self.cb5 = 0
            
    def cb6Checked(self):
        if self.cb6 == 0:
            self.cb6 = 1
        else:
            self.cb6 = 0                                                    
            
    def led1Changed(self,text):
        self.fileName = text
        
    def led2Changed(self,text):
        text = list(text.split(","))
        self.topic = text
        
    def rbn1Checked(self):
        self.status = 2
        
    def rbn2Checked(self):
        self.status = 1
        
    def rbn3Checked(self):
        
        self.status = 0
        
    def btn1Clicked(self):
        self.a = download(self.fileName,self.topic,self.status,self.cb1,self.cb2,self.cb3,self.cb4,self.cb5,self.cb6)
        if self.a == "success":
            QMessageBox.about(self,"다운 완료","다운로드 성공")
        elif self.a == "fail":
            QMessageBox.about(self,"다운 실패","다운로드 실패")
        #깃허브에 이런 소스코드들이 즐비하다 가져와 쓰는것을 추천
        else:
            QMessageBox.about(self,"실패","잘못된 형식")
            
            
    # def btn2Clicked(self):
    #     btnQa = QMessageBox.information(
    #         self,
    #         "삭제",
    #         "자료를 삭제하시겠습니까?",
    #         QMessageBox.Yes | QMessageBox.Cancel
    #     )
    #     if btnQa == QMessageBox.Yes:
    #         QMessageBox.about(self,"삭제완료","자료삭제완료")
    #     else:
    #         QMessageBox.about(self,"삭제미완료","자료삭제미완료")
app = QApplication(sys.argv)
window = MyWindow()
window.show()
app.exec_()
# pyinstaller -w -F 01.py