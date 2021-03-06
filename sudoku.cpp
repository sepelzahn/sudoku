#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

const int rows = 9;
const int cols = 9;
const int vsz = rows * cols;
int x[vsz];
int beforeRoundState[vsz];
int possibleValues[vsz];
int twoPossibilities[vsz];
int sumOfOccurencesInAreaForValue[9];
int posOfSingleOccuranceOfValue[9];

int dbg_round   = (1<<0);
int dbg_reduce  = (1<<1);
int dbg_singles = (1<<2);
int dbg_sumup   = (1<<3);
int dbg_twins   = (1<<4);
int dbg_update  = (1<<5);
int dbg_check   = (1<<6);
int dbg = dbg_round ; //| dbg_twins | dbg_update;

enum Area {
	ROW,
	COL,
	SQR
};

enum SudokuIterator {
	BEGIN,
	END,
	INCREMENT
};

bool checkForTwoTimesSameTwoPossibleValuesConstraint(int, int);
bool checkArea(int pos, Area area, bool (*)(int, int, int, int, int, Area, bool, bool*));
bool updatePossibleValuesInOtherCellsBelongingToAreasOfSolvedCell(int, int);

string areaType[] = {"row", "col", "sqr"};

int getAreaIterator(Area area, int pos, SudokuIterator loopVar){
	switch( area ){
	case Area::ROW:
		switch( loopVar ){
		case SudokuIterator::BEGIN:
			return (pos/9)*9;
			break;
		case SudokuIterator::END:
			return (pos/9)*9+9;
			break;
		case SudokuIterator::INCREMENT:
			return 1;
			break;
		default:
			cerr << "ERROR in getAreaIterator: invalid SudokuIterator " << loopVar << " used" << endl;
			return 0;
		}
		break;
	case Area::COL:
		switch( loopVar ){
		case SudokuIterator::BEGIN:
			return pos%9;
			break;
		case SudokuIterator::END:
			return vsz;
			break;
		case SudokuIterator::INCREMENT:
			return 9;
			break;
		default:
			cerr << "ERROR in getAreaIterator: invalid SudokuIterator " << loopVar << " used" << endl;
			return 0;
		}
		break;
	case Area::SQR:
		switch( loopVar ){
		case SudokuIterator::BEGIN:
			return (pos/27)*27 + ((pos%9)/3)*3;
			break;
		case SudokuIterator::END:
			return (pos/27)*27 + ((pos%9)/3)*3 + 18 + 3;
			break;
		case SudokuIterator::INCREMENT:
			return 0;
			break;
		default:
			cerr << "ERROR in getAreaIterator: invalid SudokuIterator " << loopVar << " used" << endl;
			return 0;
		}
		break;
	default:
		cerr << "ERROR in getAreaIterator: invalid Area " << area << " used" << endl;
		return 0;
	}
}

string possVals(int flags){
	string numList;
	bool first = true;
	for( int i = 0; i < 9; i++){
		if( flags & (1<<i) ){
			if( first ){
				first = false;
				numList.append(to_string(i+1));
			} else {
				numList.append(", ").append(to_string(i+1));
			}
		}
	}
	return numList;
}

bool sumUpPossibleValuesInArea( int p, int pos, int pStart, int pEnd, int pInc, Area area, bool reevaluate, bool* doneEnough){
	if( !x[p] && possibleValues[p] ){
		for( int i = 0; i < 9; i++){
			if( (possibleValues[p] & (1<<i)) ){
				sumOfOccurencesInAreaForValue[i]++;
				if( sumOfOccurencesInAreaForValue[i] == 1 ){
					if(dbg & dbg_sumup) cout << "sumUpPossibleValuesInArea(p=" << p << ", pos=" << pos << ", area=" << boolalpha << area << "): found first occurence of value " << i+1 << " at " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ")" << endl;
					posOfSingleOccuranceOfValue[i] = p;
				} else {
					if(dbg & dbg_sumup) cout << "sumUpPossibleValuesInArea(p=" << p << ", pos=" << pos << ", area=" << boolalpha << area << "): found another occurence of value " << i+1 << " at " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ")" << endl;
					posOfSingleOccuranceOfValue[i] = 0;
				}
			}
		}
	} else {
		if( x[p] && possibleValues[p] ){
			if(dbg & dbg_sumup) cout << "sumUpPossibleValuesInArea(p=" << p << ", pos=" << pos << ", area=" << boolalpha << area << "): found a solved cell with value " << x[p] << " and possibleValues = " << possVals(possibleValues[p]) << " at " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ")" << endl;
		} else if( !x[p] && !possibleValues[p] ){
			if(dbg & dbg_sumup) cout << "sumUpPossibleValuesInArea(p=" << p << ", pos=" << pos << ", area=" << boolalpha << area << "): found a not solved cell with no possibleValues = " << possibleValues[p] << " at " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ")" << endl;
		} else {
			if(dbg & dbg_sumup) cout << "sumUpPossibleValuesInArea(p=" << p << ", pos=" << pos << ", area=" << boolalpha << area << "): found a solved cell with value " << x[p] << " and no possibleValues = " << possibleValues[p] << " at " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ")" << endl;
		}
	}
	return reevaluate;
}

bool checkSingles(int pos, Area area, bool reevaluate){
	for(int i = 0; i<9; i++){
		sumOfOccurencesInAreaForValue[i] = 0;
		posOfSingleOccuranceOfValue[i] = 0;
	}
	if(dbg & dbg_singles) cout << "checkSingles(" << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << "), " << areaType[area] << ", " << boolalpha << reevaluate << ")" << endl;
	reevaluate = checkArea(pos, area, sumUpPossibleValuesInArea)?true:reevaluate;
	for(int i = 0; i<9; i++){
		if( sumOfOccurencesInAreaForValue[i] == 1 ){
			bool singleValueFound = true;
			for(int j = 0; j<9; j++){
				if( j == i ) continue;
				// find more than one single values in a cell: impossible I think, but better prove it
				if( posOfSingleOccuranceOfValue[j] == posOfSingleOccuranceOfValue[i] ){
					singleValueFound = false;
					if(dbg & dbg_singles) cout << "checkSingles(" << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << "), " << areaType[area] << ", " << boolalpha << reevaluate << "): found single occurance in area for values "<< i+1 << " and " << j+1 << " in same cell " << posOfSingleOccuranceOfValue[j] << " (" << (posOfSingleOccuranceOfValue[j])%9+1 << "/" << (posOfSingleOccuranceOfValue[j])/9+1 << ") having possible values of " << possVals(possibleValues[posOfSingleOccuranceOfValue[j]]) << endl;
					break;
				}
			}
			if( singleValueFound == false ) {
				continue;
			} else {
				int possibleValuesBefore = possibleValues[posOfSingleOccuranceOfValue[i]];
				x[posOfSingleOccuranceOfValue[i]] = i+1;
				possibleValues[posOfSingleOccuranceOfValue[i]] = 0;
				twoPossibilities[posOfSingleOccuranceOfValue[i]] = 0;
				updatePossibleValuesInOtherCellsBelongingToAreasOfSolvedCell(posOfSingleOccuranceOfValue[i], x[posOfSingleOccuranceOfValue[i]]);
				reevaluate = true;
				if(dbg & dbg_singles) cout << "checkSingles(" << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << "), " << areaType[area] << ", " << boolalpha << reevaluate << "): found single occurance in area for value "<< i+1 << " in cell " << posOfSingleOccuranceOfValue[i] << " (" << (posOfSingleOccuranceOfValue[i])%9+1 << "/" << (posOfSingleOccuranceOfValue[i])/9+1 << "): reduce from possible values of " << possVals(possibleValuesBefore) << " to " << i+1 << endl;
			}
		}
	}
	return reevaluate;
}

bool reducePossibleValues( int p, int pos, int pStart, int pEnd, int pInc, Area area, bool reevaluate, bool* doneEnough){
	if( x[p] && p != pos ){
		int theOnlyOneValue = 0;
		int impossibleCount = 0;
		int before = possibleValues[pos];

		if( !before ) return reevaluate;

		possibleValues[pos] &= ~(1<<(x[p]-1));
		reevaluate = (before==possibleValues[pos])?reevaluate:true;

		for( int i = 0; i < 9; i++){
			if( (possibleValues[pos] & (1<<i)) ){
				theOnlyOneValue = i+1;
			} else {
				impossibleCount++;
			}
		}

		if( impossibleCount > 7 ){
			if( theOnlyOneValue > 0 ){
				x[pos] = theOnlyOneValue;
				possibleValues[pos] = 0;
				if(dbg & dbg_reduce) cerr << "solved by deleting value " << x[p] << " in " << areaType[area] << "-area in cell " << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << ") = " << x[pos] << ", because cell " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ") claims the value. Possible values of cell " << pos << " are now " << possVals(possibleValues[pos]) << endl;
			} else {
				if(dbg & dbg_reduce) cerr << "not solved although impossibleCount > 7: " << impossibleCount << ", where value = " << x[pos] << " in " << areaType[area] << "-area in cell " << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << "), cell " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ") claims the value. Possible values of cell " << pos << " changed from " << possVals(possibleValues[pos]) << " to " << possVals(before) << endl;
				;
			}
		} else {
			if( before == possibleValues[pos] ){
				if(dbg & dbg_reduce) cerr << "nothing to do: value " << x[p] << " in " << areaType[area] << "-area already deleted in cell " << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << "), cell " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ") claims the value. Possible values of cell " << pos << " are now " << possVals(possibleValues[pos]) << endl;
			} else {
				if(dbg & dbg_reduce) cerr << "possible values reduced by deleting value " << x[p] << " in " << areaType[area] << "-area in cell " << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << ") because cell " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ") claims the value. Possible values of cell " << pos << " are now " << possVals(possibleValues[pos]) << endl;
			}
		}
	}
	return reevaluate;
}

bool checkForTwoTimesSameTwoPossibleValuesConstraint(int pos, int posInSameArea) {
	bool reevaluate = false;
	int theOnlyOneValue = 0;
	int impossibleCount = 0;

	if( !x[posInSameArea] && posInSameArea != pos && possibleValues[posInSameArea] != twoPossibilities[pos] ){
		int before = possibleValues[posInSameArea];
		possibleValues[posInSameArea] &= ~twoPossibilities[pos];
		if(dbg & dbg_twins) cout << "possible value emliminated from cell " << posInSameArea << " (" << posInSameArea%9+1 << "/" << posInSameArea/9+1 << "): before: '" << possVals(before) << "', after: '" << possVals(possibleValues[posInSameArea]) << "', where the TwoTimesTwo-Template is " << pos << " (" << pos%9+1 << "/" << pos/9+1 << ") with possibleValues = '" << possVals(twoPossibilities[pos]) << "'" << endl;

		for( int i = 0; i < 9; i++){
			if( (possibleValues[posInSameArea] & (1<<i)) ){
				theOnlyOneValue = i+1;
			} else {
				impossibleCount++;
			}
		}

		if( impossibleCount > 7 ){
			reevaluate = true;
			if(dbg & dbg_twins) cout << "solved cell " << posInSameArea << " (" << (posInSameArea)%9+1 << "/" << (posInSameArea)/9+1 << ") = " << theOnlyOneValue << endl;
			x[posInSameArea] = theOnlyOneValue;
			possibleValues[posInSameArea] = 0;
			twoPossibilities[posInSameArea] = 0;
			updatePossibleValuesInOtherCellsBelongingToAreasOfSolvedCell(posInSameArea, x[posInSameArea]);
		} else {
			if( before != possibleValues[posInSameArea] ) reevaluate = true;
		}
	}
	return reevaluate;
}

bool areThereTwoSameTwoPossibleValues( int p, int pos, int pStart, int pEnd, int pInc, Area area, bool reevaluate, bool* doneEnough){
	if( p != pos && twoPossibilities[pos] == twoPossibilities[p] ){
		if(dbg & dbg_twins) cout << "same possible numbers in " << areaType[area] << "-area in cell " << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << ") and cell " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ") = " << possVals(twoPossibilities[p]) << endl;
		for( int o = pStart ; o < pEnd ; o += ((pInc>0)?pInc:((o%9==pStart%9+2)?7:1)) ){
			reevaluate = checkForTwoTimesSameTwoPossibleValuesConstraint(pos, o)?true:reevaluate;
		}
		*doneEnough = true;
	}
	return reevaluate;
}

bool updatePossibleValuesInOtherCellsBelongingToAreasOfSolvedCell(int solvedCellPos, int solvedValue){
	int pStart, pEnd, pInc;
	for( int area = Area::ROW; area <= Area::SQR; area++ ){
		pStart = getAreaIterator((Area) area, solvedCellPos, SudokuIterator::BEGIN);
		pEnd   = getAreaIterator((Area) area, solvedCellPos, SudokuIterator::END);
		pInc   = getAreaIterator((Area) area, solvedCellPos, SudokuIterator::INCREMENT);
		for( int p = pStart ; p < pEnd ; p += ((pInc>0)?pInc:((p%9==pStart%9+2)?7:1)) ){
			if( p != solvedCellPos && possibleValues[p] ){
				if( possibleValues[p] == (1<<(solvedValue-1)) ){
					if(dbg & dbg_update) cout << "updatePossibleValuesInOtherCellsBelongingToAreasOfSolvedCell: cannot reduce solvedValue " << solvedValue << " from possibleValues[" << p << "] = '" << possVals(possibleValues[p]) << "', because the value of the solved cell " << solvedCellPos << " (" << (solvedCellPos)%9+1 << "/" << (solvedCellPos)/9+1 << ") = " << solvedValue << " colides with the single value '" << possVals(possibleValues[p]) << "' left in the list of possible values of cell " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ")" << " in " << areaType[area] << "-area" << endl;
				} else {
					int possibleValuesBefore = possibleValues[p];
					possibleValues[p] &= ~(1<<(solvedValue-1));
					twoPossibilities[p] = 0;
					if(dbg & dbg_update) cout << "updatePossibleValuesInOtherCellsBelongingToAreasOfSolvedCell: reduced solvedValue " << solvedValue << " from possibleValues[" << p << "] = '" << possVals(possibleValuesBefore) << "', getting new list '" << possVals(possibleValues[p]) << "', because the value of the solved cell " << solvedCellPos << " (" << (solvedCellPos)%9+1 << "/" << (solvedCellPos)/9+1 << ") = " << solvedValue << " requires this change in the possible values of cell " << p << " (" << (p)%9+1 << "/" << (p)/9+1 << ")" << " in " << areaType[area] << "-area" << endl;
				}
			}
		}
	}
}

bool checkArea(int pos, Area area, bool (*fP)(int, int, int, int, int, Area, bool, bool*)){
	int pStart, pEnd, pInc;
	bool reevaluate = false;

	pStart = getAreaIterator(area, pos, SudokuIterator::BEGIN);
	pEnd   = getAreaIterator(area, pos, SudokuIterator::END);
	pInc   = getAreaIterator(area, pos, SudokuIterator::INCREMENT);
	if(dbg & dbg_check) cout << "checkArea: pos=" << pos << ", area=" << areaType[area] << ", pStart=" << pStart << ", pEnd=" << pEnd << ", pInc=" << pInc << " (" << (pos)%9+1 << "/" << (pos)/9+1 << ")" << endl;
	
	for( int p = pStart ; p < pEnd ; p += ((pInc>0)?pInc:((p%9==pStart%9+2)?7:1)) ){
		bool doneEnough = false;
		reevaluate = fP( p, pos, pStart, pEnd, pInc, area, reevaluate, &doneEnough);
		if( doneEnough ){
			break;
		}
	}
	return reevaluate;
}

int main(int argc, char**argv){
	ifstream sudokuPlan;
	char linebuf[vsz+1];
	int xfound;
	int round = 0;

	int bytesRead = 0;
	int bytesReadAgain = 0;
	int found = 0;
	bool reevaluate = true;
	bool stop = false;
	int opt;


	while ((opt = getopt(argc, argv, "d:")) != EOF){
        	switch(opt){
			case 'd': 
				dbg = atoi(optarg);
				break;
			case '?':
				cout << "usuage " << argv[0] << " -d <debuglevel>" << endl;
			default:
				exit(1);
        	}
	}
	if( optind < argc ){
		if( sudokuPlan = ifstream(argv[optind]) ){
			cin.rdbuf(sudokuPlan.rdbuf());
		} else {
			cout << "Error loading file " << argv[optind] << endl;
			exit(1);
		}
	} else {
		cout << "Reading Sudoku plan from stdin (abort with Ctrl c): type in " << vsz << " digits from 0..9 in one or more lines to make up a " << rows << "x" << cols << " cells Sudoku plan, where 0 (or blank) marks an empty cell: " << endl;
	}

	for( int i = 0; i < vsz+1 ; i++) linebuf[i] = '\0';

	cin.get(linebuf, vsz+1);
	bytesRead = cin.gcount();
	cout << boolalpha;
	cout << "\n\nRead a line with " << bytesRead << " bytes: " << linebuf << endl;
	while( bytesRead < vsz ){
		if( !cin.good() ){
			cout << "\nios state: " << "good = " << cin.good() << ", eof = " << cin.eof() << ", fail = " << cin.fail() << ", bad = " << cin.bad() << endl;
			if( cin.eof() ) break;
			if( cin.bad() ) exit(1);
			cin.clear();  // cin.fail() == empty line
		}
		cin.ignore(1,'\n');
		if( !cin.good() ) break;
		cout << "Enter next line at pos " << bytesRead << ", still missing " << vsz - bytesRead << " bytes (good = " << cin.good() << "): ";
		cin.get(linebuf + bytesRead, vsz+1 - bytesRead);
		if( !cin.good() ){
			cout << "\nios state: " << "good = " << cin.good() << ", eof = " << cin.eof() << ", fail = " << cin.fail() << ", bad = " << cin.bad() << endl;
			if( cin.eof() ) break;
			if( cin.bad() ) exit(1);
			cin.clear();  // cin.fail() == empty line
		}
		if( !cin.good() ) break;
		bytesReadAgain = cin.gcount();
		cout << "\nRead a line with " << bytesReadAgain << " bytes: " << linebuf+bytesRead << endl;
		bytesRead += bytesReadAgain;
	}

	for( int pos = 0; pos < vsz; pos++ ){
		if( pos >= bytesRead || linebuf[pos] > '9' || linebuf[pos] < '1'){
			x[pos] = 0;
			possibleValues[pos] = 511 ; // 0b111111111: start with all values [1..9] possible
		} else {
			x[pos] = linebuf[pos] - '0';
			possibleValues[pos] &= ~(1<<(x[pos]-1));
		}
	}

	cout << setfill('-') ;
	while( reevaluate ){
		while( reevaluate ){
			round++;
			reevaluate = false;

			for( int pos = 0; pos < vsz; pos++ ){
				twoPossibilities[pos] = 0;
			}
		
			for( int pos = 0; pos < vsz; pos++ ){
				beforeRoundState[pos] = x[pos];
			}
		
			for( int pos = 0; pos < vsz; pos++ ){
				if( !(pos % 9) ) cout << endl << setw(91) << '-' << endl << '|';
				if( x[pos] ){
					// The value of the cell is already determined, so print it
					cout << "    " << x[pos] << "    |";
				} else {
					// The value of the cell is not determined, so calculate possible values.
					// For each cell the possible values are stored in the 9 least significant bits
					// in an integer of an array storing such an integer for each cell: possibleValues[cell index in x].
					// Bit position 0..8 represent each a bool for value 1..9 for the given cell: Is a value possible due to 
					// constraints in column, row and 3x3 area? Bit set: yes, bit not set: no 


					// Checking the column the actual cell resides in for constraints.
					// Only a cell with already determined value -> "if( x[p] )" is considered as constraint 
					// (i.e. a value the actual cell cannot anymore be assigned to, because it is already in use)
					reevaluate = checkArea(pos, Area::COL, reducePossibleValues)?true:reevaluate;

					// Checking the row the actual cell resides in for constraints.
					reevaluate = checkArea(pos, Area::ROW, reducePossibleValues)?true:reevaluate;

					// Checking the 3x3 area (row by row) the actual cell resides in for constraints.
					reevaluate = checkArea(pos, Area::SQR, reducePossibleValues)?true:reevaluate;

					// If just one possible value is left in list of possible values then this cell is solved
					if( x[pos] ){
						cout << "    " << x[pos] << "    |";
					} else {
						int possibleCount = 0;
						for( int i = 0; i < 9; i++){
							if( possibleValues[pos] & (1<<i) ){
								// print all possible digits
								cout << i+1;
								possibleCount++;
							} else {
								cout << '.';
							}
						}
						cout << '|';
						if( possibleCount == 2 ){
							//cout << "two at " << pos << " (" << (pos)%9+1 << "/" << (pos)/9+1 << ") = " << possVals(possibleValues[pos]) << endl;
							twoPossibilities[pos] = possibleValues[pos];
						}
					}
				}
			}
			cout << endl << setw(91) << '-' << endl << endl;
			if(dbg & dbg_round){
				 cout << endl << "re-evaluation necessary after round " << round << " ? " << boolalpha << reevaluate << endl;

				int changedCount = 0;
				for( int pos = 0; pos < vsz; pos++ ){
					if( beforeRoundState[pos] != x[pos] ){
						changedCount++;
					}
				}
				cout << "solved " << changedCount << " cells in round " << round << ": ";
				for( int pos = 0; pos < vsz; pos++ ){
					if( beforeRoundState[pos] != x[pos] ){
						cout << "cell " << pos << " (" << pos%9+1 << "/" << pos/9+1 << ") = " << x[pos] << ", ";
					}
				}
				cout << endl;
			}
		}
		if( stop ) return 0;
	
		for( int pos = 0; pos < vsz; pos++ ){
			if( twoPossibilities[pos] ){
				reevaluate = checkArea(pos, Area::ROW, areThereTwoSameTwoPossibleValues)?true:reevaluate;
				reevaluate = checkArea(pos, Area::COL, areThereTwoSameTwoPossibleValues)?true:reevaluate;
				reevaluate = checkArea(pos, Area::SQR, areThereTwoSameTwoPossibleValues)?true:reevaluate;
			}
		}
		if( reevaluate ) continue;


		for( int pos = 0; pos < vsz; pos+=9 ){
			reevaluate = checkSingles(pos, Area::ROW, reevaluate)?true:reevaluate;
		}
		for( int pos = 0; pos < 9; pos++ ){
			reevaluate = checkSingles(pos, Area::COL, reevaluate)?true:reevaluate;
		}
		for( int pos = 0; pos < vsz; pos+=((pos%9==6)?21:3) ){
			reevaluate = checkSingles(pos, Area::SQR, reevaluate)?true:reevaluate;
		}
	}
	return 0;
}
