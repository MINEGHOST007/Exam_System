#include "../Templates/template.h"
#include <sys/socket.h>
#include <algorithm> // Add this line to include the <algorithm> header
#include <random>

// inet_addr
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>
#define PORT 8080
using namespace std;

// ------------------ for leaderboard ---------------
struct Student_Result {
    string roll;
    int marks;
};

struct Topic_Result {
    string name;
    string accuracy;
    string roll;
};

bool compareByMarks(const Student_Result& a, const Student_Result& b) {
    return a.marks > b.marks;
}
// --------------------------------------------------

Question::Question()
{
    questionBank.clear();
}

void Question::insertQuestion(QuestionInfo* question)
{
    this->questionBank.push_back(question);
}

ResultData Question::startExam(int newSocket)
{
    ResultData result;
    result.marksObtained = 0;
    int code;

    if(questionBank.empty())
    {
        code = EMPTY_QUESTIONBANK_CODE;
        send(newSocket, &code, sizeof(code), 0);
        result.marksObtained = -1;
        return result;
    }

    std::map<std::string, int> topicCorrectAnswers;
    std::map<std::string, int> totalTopicQuestions;

    for(const auto &q : questionBank)
    {
        totalTopicQuestions[q->tags]++;
    }

    for(size_t i = 0; i < questionBank.size(); i++)
    {
        code = RECIEVE_QUESTION_CODE;
        send(newSocket, &code, sizeof(code), 0);

        StudentQuestion question;
        strcpy(question.que, questionBank[i]->que);
        strcpy(question.opt1, questionBank[i]->opt1);
        strcpy(question.opt2, questionBank[i]->opt2);
        strcpy(question.opt3, questionBank[i]->opt3);
        strcpy(question.opt4, questionBank[i]->opt4);
        strcpy(question.marks, questionBank[i]->marks);

        send(newSocket, &question, sizeof(question), 0);

        char answer[5];
        recv(newSocket, &answer, sizeof(answer), 0);

        std::string topic = questionBank[i]->tags;
        if(answer[0] == questionBank[i]->answer[0])
        {
            result.marksObtained += atoi(questionBank[i]->marks);
            topicCorrectAnswers[topic]++;
        }
    }

    for(const auto &entry : totalTopicQuestions)
    {
        const std::string &topic = entry.first;
        int totalQuestions = entry.second;
        int correctAnswers = topicCorrectAnswers[topic];
        int accuracy = (correctAnswers * 100) / totalQuestions;
        result.topicAccuracy[topic] = accuracy;
    }

    code = END_EXAM_CODE;
    send(newSocket, &code, sizeof(code), 0);
    sleep(1);
    send(newSocket, &result.marksObtained, sizeof(result.marksObtained), 0);
    sleep(1);

    return result;
}

void Question::sendQuestions(int newSocket)
{
    int code;
    if(questionBank.size() == 0)
    {
        code = EMPTY_QUESTIONBANK_CODE;
        send(newSocket, &code, sizeof(code),0);
        return;
    }
    for(size_t i=0; i < questionBank.size(); i++)
    {
        code = SEE_QUESTION_CODE;
        send(newSocket,&code,sizeof(code),0);
        QuestionInfo *question = new QuestionInfo;
        strcpy(question->que, questionBank[i]->que);
        strcpy(question->opt1,questionBank[i]->opt1);
        strcpy(question->opt2,questionBank[i]->opt2);
        strcpy(question->opt3,questionBank[i]->opt3);
        strcpy(question->opt4,questionBank[i]->opt4);
        strcpy(question->answer, questionBank[i]->answer);
        strcpy(question->marks,questionBank[i]->marks);
        strcpy(question->tags, questionBank[i]->tags);
        send(newSocket, question,sizeof(* question),0);
    }
    code = END_QUESTION_SEEING_CODE;
    send(newSocket, &code, sizeof(code), 0);
    return ;
}


void Question::shuffleQuestions() {
    srand(unsigned(time(0))); 
    shuffle(questionBank.begin(), questionBank.end(), default_random_engine(random_device{}()));
}

void server_side_student_registration(int newSocket)
{
    string uname, password, rollno, department;
    StudentUserInfo *newUserInfo = new StudentUserInfo;

    recv(newSocket, newUserInfo, sizeof(*newUserInfo), 0);
    uname = newUserInfo->username;
    password = newUserInfo->password;
    rollno = newUserInfo->rollno;
    department = newUserInfo->department;
    cout << "Received info..\n";
    fstream file;
    file.open("student_database.txt", ios::app);
    file << rollno << "|" << password << "|" << uname << "|" << department << "|"<<endl;
    file.close();
    return;
}

void server_side_teacher_registration(int newSocket)
{
    string uname, password, teacher_id, department;
    TeacherUserInfo *newUserInfo = new TeacherUserInfo;
    recv(newSocket, newUserInfo, sizeof(*newUserInfo), 0);

    uname = newUserInfo->username;
    password = newUserInfo->password;
    department = newUserInfo->department;
    teacher_id = newUserInfo->teacherid;

    fstream file;
    file.open("teacher_database.txt", ios::app);
    file << teacher_id << "|" << password << "|" << uname << "|" << department<<"|"<<endl;
    file.close();
    return;
}

void server_side_login(int newSocket)
{
    cout<<"login started.."<<endl;
    string file_name;
    char usertype;
    recv(newSocket,&usertype,sizeof(usertype),0);
    
    if(usertype=='S')
    {
        file_name = "student_database.txt";
    }
    else
    {
        file_name = "teacher_database.txt";
    }

    loginInfo *userInfo = new loginInfo;
    int code;

    ifstream file(file_name,  ios::in);
    if (file.is_open())
    {
        while (1)
        {
            file.clear();
            file.seekg(0,ios_base::beg);
            recv(newSocket, userInfo, sizeof(*userInfo), 0);
            bool not_found = true;
             string line;
            while ( getline(file, line))
            {
                string attr;
                stringstream str(line);
                getline(str,attr,'|');
                if(attr == userInfo->id)
                {
                    getline(str,attr,'|');
                    if(attr == userInfo->password)
                    {
                        not_found = false;
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if(not_found)
            {
                code = LOGIN_FAIL_CODE;
                send(newSocket,&code,sizeof(code),0);
            }
            else
            {
                break;
            }

        }
        file.close();
    }
    else
    {
        code = SERVER_ERROR_CODE;
        send(newSocket, &code ,sizeof(code),0);
    }
    return;
}

void setQuestion(int newSocket, string department, Question &deptObj)
{
    
    string fileName = department + ".txt";
    fstream file;
    file.open(fileName,ios::app);
    while(1)
    {
        int code;
        recv(newSocket,&code, sizeof(code),0);
        if(code == SET_QUESTION_CODE)
        {
            QuestionInfo* question = new QuestionInfo;
            recv(newSocket, question, sizeof(* question),0);
            deptObj.insertQuestion(question);
            file<<question->que<<"|"<<question->opt1<<"|"<<question->opt2<<"|"<<question->opt3<<"|"<<question->opt4<<"|"<<question->answer<<"|"<<question->marks<<"|"<<question->tags<<"|"<<endl;
        }
        else
        {
            break;
        }
    }
    file.close();
    
}

void addQuestionFromFile(string department,Question& deptobj)
{
    string fileName = department+".txt";
    ifstream file(fileName,ios::in);
    if(file.is_open())
    {
        string line;
        while(getline(file,line))
        {
            string attr;
            stringstream str(line);
            QuestionInfo* question = new QuestionInfo;
            getline(str,attr,'|');
            strcpy(question->que,attr.c_str());
            getline(str,attr,'|');
            strcpy(question->opt1,attr.c_str());
            getline(str,attr,'|');
            strcpy(question->opt2,attr.c_str());
            getline(str,attr,'|');
            strcpy(question->opt3,attr.c_str());
            getline(str,attr,'|');
            strcpy(question->opt4,attr.c_str());
            getline(str,attr,'|');
            strcpy(question->answer,attr.c_str());
            getline(str,attr,'|');
            strcpy(question->marks,attr.c_str());
            getline(str, attr, '|');
            strcpy(question->tags, attr.c_str());
            deptobj.insertQuestion(question);
        }
    }
    return;
}


void updateResult(string id,string department, int marksObtained)
{
    string fileName = department+"_result.txt";
    fstream file;
    file.open(fileName,ios::app);
    file<<id<<"|"<<marksObtained<<"|"<<endl;
    file.close();
}

void updateTopicResult(string topic_name,string department, string accuracy, string id)
{
    string fileName = department+"_topic_result.txt";
    fstream file;
    file.open(fileName,ios::app);
    file<<topic_name<<"|"<<accuracy<<"|"<<id<<"|"<<endl;
    file.close();
}


vector<Student_Result> sortResultFile(string &fileName)
{
    ifstream inFile(fileName);
    vector<Student_Result> students;

    if (inFile.is_open()) {
         string line;
        while ( getline(inFile, line)) {
            Student_Result student;
            stringstream str(line);
            string attr;
            getline(str,attr,'|');
            student.roll = attr;
            getline(str,attr,'|');

            student.marks = stoi(attr);
            students.push_back(student);
        }

        inFile.close();

        // Sort the students based on marks
         sort(students.begin(), students.end(), compareByMarks);

    } else {
         cout << "Error opening input file\n";
    }

    return students;
}

void getLeaderboard(int newSocket, string dept)
{
    string fileName = dept+"_result.txt";
    vector<Student_Result> students = sortResultFile(fileName);
    int code;
    if(students.size())
    {
        for(auto it:students)
        {
            code = LEADERBOARD_CODE;
            send(newSocket, &code , sizeof(code),0);
            leaderboardInfo* leaderboard = new leaderboardInfo;
            strcpy(leaderboard->id, it.roll.c_str());
            strcpy(leaderboard->marks, (to_string(it.marks)).c_str());
            send(newSocket,leaderboard,sizeof(* leaderboard), 0);
        }
    }
    else
    {
        code = SERVER_ERROR_CODE;
        send(newSocket, &code, sizeof(code), 0);
        cout<<"Error getting the leaderboard\n";
        return;
    }
    code = END_OF_LEADERBOARD_CODE;
    send(newSocket,&code , sizeof(code),0);
    return ;
    
}

vector<Topic_Result> sortTopicResultFile(string &fileName)
{
    ifstream inFile(fileName);
    vector<Topic_Result> topics;

    if (inFile.is_open()) {
         string line;
        while ( getline(inFile, line)) {
            Topic_Result topic;
            stringstream str(line);
            string attr;
            getline(str,attr,'|');
            topic.name = attr;
            getline(str,attr,'|');
            topic.accuracy = attr;
            getline(str,attr,'|');
            topic.roll = attr;
            topics.push_back(topic);
        }

        inFile.close();

    } else {
         cout << "Error opening input file\n";
    }

    return topics;
}

void getTopicLeaderboard(int newSocket, string dept)
{
    string fileName = dept+"_topic_result.txt";
    vector<Topic_Result> topics = sortTopicResultFile(fileName);
    int code;
    if(topics.size())
    {
        for(auto it:topics)
        {
            code = TOPIC_LEADERBOARD_CODE;
            send(newSocket, &code , sizeof(code),0);
            topicleaderboardInfo* leaderboard = new topicleaderboardInfo;
            strcpy(leaderboard->topic_name, it.name.c_str());
            strcpy(leaderboard->count, it.accuracy.c_str());
            strcpy(leaderboard->id, it.roll.c_str());
            send(newSocket,leaderboard,sizeof(* leaderboard), 0);
        }
    }
    else
    {
        code = SERVER_ERROR_CODE;
        send(newSocket, &code, sizeof(code), 0);
        cout<<"Error getting the leaderboard\n";
        return;
    }
    code = END_TOPIC_LEADERBOARD_CODE;
    send(newSocket,&code , sizeof(code),0);
    return ;
}