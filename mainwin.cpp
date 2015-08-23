/*
* Mainwin.cpp
* Idean and Henry
* TODO:
* 1. tempSet is of type Set<WebPage> but should be Set<WebPage*> for efficiency.
*
*
*
*
*
*/


#include "mainwin.h"
#include <sstream>
#include <string>
#include <exception>
#include <iostream>
#include <iomanip>

using namespace std;

MainWin::MainWin(QWidget *parent) : QMainWindow(parent)
{

	// allocate memory for all widgets
	winLayout     = new QGridLayout;
	txtWordsInput = new QLineEdit;
	outputList    = new QListWidget;
	btnSearch     = new QPushButton("&Search");
	btnAbout      = new QPushButton("&About");
	btnQuit	      = new QPushButton("&Quit");

	// setup the grid layout and make it fill the window
	centerWidget  = new QWidget(this);
	centerWidget->setLayout(winLayout);
	this->setCentralWidget(centerWidget);

	// Set Search Button to be default
	btnSearch->setDefault(true);

	// add all widgets to the layout
	// redo the following 4 lines to customize the layout
	winLayout->addWidget(txtWordsInput,	1,1,1,5);
	winLayout->addWidget(outputList,	2,1,3,7);
	winLayout->addWidget(btnSearch,     1,6,1,1);
	winLayout->addWidget(btnAbout,     	5,5,1,1);
	winLayout->addWidget(btnQuit,	   	5,6,1,1);

	// connect clicked signal of each button to the right slot
	connect(btnSearch,	SIGNAL(clicked()),	this, SLOT(doLookupWord()));
	connect(btnAbout,      	SIGNAL(clicked()),	this, SLOT(showAbout()));
	connect(btnQuit, 	SIGNAL(clicked()),	this, SLOT(quitButton()));
	connect(outputList, SIGNAL(itemDoubleClicked(QListWidgetItem *)) , this, SLOT(lookupPageListItem(QListWidgetItem *)));


	readInWebsites();



}


int MainWin::getIDfromName(std::string filename) {
	for(auto p : WebPageList) {
		if (p->filename().compare(filename) == 0) {
			return p->pageID;
		}
	}
	return -1;
}


void MainWin::readInWebsites() {
	ifstream inFile;
	inFile.open("data/index.txt");
	string file;
	if (inFile.good())
	{
		while (!inFile.eof())
		{
			inFile >> file;
			WebPage * webpage = new WebPage(file);
			WebPageList.insert(WebPageList.begin(), webpage);
		}
	}
	else
	{
		exit(1);
	}

	for (auto p : WebPageList)
	{
		Set<string> page = p->allWords();

		for (auto setVal : page)
		{
			try
			{
				myMap.insert(pair<string, Set<WebPage *> >(setVal, Set<WebPage *>()));
			}
			catch (logic_error & )
			{
				//Do nothing
				//cout << "Key already exist" << endl;
			}
			myMap[setVal].insert(p); 
		}

		Set<string> outLinks = *(p->allOutgoingLinks());
		for (auto it : outLinks)
		{
			for (auto itPage : WebPageList)
			{					
				if(itPage->filename().compare(it) == 0) 
				{
					WebPage &wp = const_cast<WebPage&>(*itPage);
					//cout << p->filename() << " linking to " << wp->filename() << endl;
					//checking the links to each WebPage
					wp.addIncomingLink(p->filename());
				}
			}
		}
	}
}

MainWin::~MainWin()
{
	for(auto i : WebPageList) {
		delete i;
	}
	clear();
}

void MainWin::clear()
{
	delete winLayout;
	delete txtWordsInput;
	delete outputList;
	delete btnSearch;
	delete btnAbout;
	delete btnQuit;
	delete centerWidget;

}


void MainWin::doLookupWord()
{
	outputList->clear();

	std::string textInput = "";
	textInput = txtWordsInput->text().toStdString();

	stringstream ss(textInput);
	string input = "";

	ss >> input;

	if (input == "AND")
	{
		input = "";
		string temp1;	

		while (ss >> temp1)
		{
			input = input + temp1;
		}

		replace(input.begin(), input.end(), '(', ' ');
		replace(input.begin(), input.end(), ')', ' ');
		replace(input.begin(), input.end(), ',', ' ');

		stringstream ss1(input);
		while (ss1 >> temp)
		{
			std::locale loc;
			string lowerTemp1;
			for(unsigned int j = 0; j < temp.length(); j++)
			{
				lowerTemp1 = lowerTemp1 + tolower(temp[j], loc);
			}
			wordSet.insert(lowerTemp1);
		}

		try
		{
			for (Set<string>::iterator setVal2 =  wordSet.begin(); setVal2 != wordSet.end(); ++setVal2)
			{
				tempSet = myMap[*setVal2];
				++setVal2;
				if (setVal2 != wordSet.end())
				{
					tempSet2 = myMap[*setVal2];
					tempSet = tempSet.setIntersection(tempSet2);
				}
			}
			results.clear();
			for(auto setIter : tempSet) {
				results.insert(results.end(), setIter);
			}
			populateResults(results);
		}
		catch (logic_error & )
		{
			//Do nothing
			//cout << e.what() << endl;
		}
	}
	else if (input == "OR")
	{
		input = "";
		string temp2;	

		while (ss >> temp2)
		{
			input = input + temp2;
		}

		replace(input.begin(), input.end(), '(', ' ');
		replace(input.begin(), input.end(), ')', ' ');
		replace(input.begin(), input.end(), ',', ' ');

		stringstream ss2(input);
		while (ss2 >> temp)
		{
			std::locale loc;
			string lowerTemp2;
			for(unsigned int j = 0; j < temp.length(); j++)
			{
				lowerTemp2 = lowerTemp2 + tolower(temp[j], loc);
			}
			wordSet.insert(lowerTemp2);
		}

		try {
			for (Set<string>::iterator setVal2 =  wordSet.begin(); setVal2 != wordSet.end(); ++setVal2)
			{
				tempSet = myMap[*setVal2];
				++setVal2;
				if (setVal2 != wordSet.end())
				{
					tempSet2 = myMap[*setVal2];
					tempSet = tempSet.setUnion(tempSet2);
				}
			}
			if (!tempSet.empty()) {
				results.clear();
				for(auto setIter : tempSet) {
					results.insert(results.end(), setIter);
				}
				populateResults(results);
			}
			else
				cout << "Sorry, no WebPage(s) found for your search." << endl;
		}
		catch (logic_error & )
		{
			//Do nothing
			//cout << e.what() << endl;
		}

	}
	else
	{
		for(unsigned int j = 0; j < input.length(); j++)
		{
			lowerTemp = lowerTemp + tolower(input[j], loc);
		}

		input = lowerTemp;
		lowerTemp = "";
		try 
		{		
			results.clear();
			for(auto setIter : myMap[input]) {
				results.insert(results.end(), setIter);
			}
			populateResults(results);
		}
		catch (logic_error & )
		{
			//do nothing
		}
	}
}

void MainWin::populateResults(vector<WebPage *> results) {
	for (auto resultsIter : results)
	{
		QListWidgetItem *result = new QListWidgetItem(outputList);
		result->setText(QString::fromStdString(resultsIter->filename()));
	}
}

void MainWin::generatePageWindow(const WebPage * site) {
	if(site == NULL) {
		return;
	}
	PageWin* window = new PageWin(this);

	window->setWindowTitle(QString::fromStdString(site->filename()));
	window->setSource(site);
	window->show();
}

void MainWin::lookupPageListItem(QListWidgetItem * item) {
	std::string filename = item->text().toStdString();
	for (auto iter : WebPageList) {
		if (iter->filename() == filename) {
			generatePageWindow(iter);
			return;
		}
	}
}

void MainWin::showAbout()
{
	QMessageBox::information(this, tr("About"), tr("Henry and Idean's Search Engine"));
}

void MainWin::quitButton()
{
	cout << "Deleting" << endl;	
	clear();
	exit(1);
}
