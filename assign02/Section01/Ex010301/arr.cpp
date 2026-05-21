#include <iostream>
using namespace std;

int main() {

	double score[5];

	cout << "첫 번째 학생의 점수 입력" << endl;
	cin >> score[0];

	cout << "두 번째 학생의 점수 입력" << endl;
	cin >> score[1];

	cout << "세 번째 학생의 점수 입력" << endl;
	cin >> score[2];

	cout << "네 번째 학생의 점수 입력" << endl;
	cin >> score[3];

	cout << "다섯 번째 학생의 점수 입력" << endl;
	cin >> score[4];

	double sum = score[0] + score[1] + score[2] + score[3] + score[4];
	double avg = sum / 5;
	

	cout << "총점"<< sum << endl;
	cout << "평균"<< avg<< endl;


	return 0;
}