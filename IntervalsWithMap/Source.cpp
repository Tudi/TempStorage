#define _ITERATOR_DEBUG_LEVEL 2

#include <iostream>
#include <map>
#include <string>

class KeyK {
private:
	int value;

public:
	// Constructor
	KeyK(const int& val) : value(val) {}

	// Less-than operator
	bool operator<(const KeyK& other) const {
		return value < other.value;
	}

	// Copy constructor
	KeyK(const KeyK& other) : value(other.value) {}

	// Assignment operator
	KeyK& operator=(const KeyK& other) {
		if (this != &other) {
			value = other.value;
		}
		return *this;
	}

	// Overloading << operator for std::cout
	friend std::ostream& operator<<(std::ostream& os, const KeyK& key) {
		os << key.value;
		return os;
	}

	// Prefix increment operator (++obj)
/*	KeyK& operator++() {
		// Implement increment logic here
		++value;
		return *this;
	} */
	std::string toStr() const { return std::to_string(value); }
};

class ValueTypeV {
public:
	ValueTypeV() = default;

	ValueTypeV(int v) { value = v; }

	// Copy constructor
	ValueTypeV(const ValueTypeV& other) = default;

	// Assignment operator
	ValueTypeV& operator=(const ValueTypeV& other) = default;

	// Equality operator
	bool operator==(const ValueTypeV& other) const {
		// Implement equality comparison logic here
		// For simplicity, let's assume ValueTypeV only contains one member variable named 'value'
		return value == other.value;
	}

	// Inequality operator
	bool operator!=(const ValueTypeV& other) const {
		return !(*this == other);
	}

	// Output stream operator
	friend std::ostream& operator<<(std::ostream& os, const ValueTypeV& obj) {
		// Output the value of ValueTypeV object
		// For simplicity, let's assume ValueTypeV only contains one member variable named 'value'
		os << obj.value;
		return os;
	}

	std::string toStr() const { return std::to_string(value); }

private:
	// Define member variables here, if any
	// For simplicity, let's assume ValueTypeV only contains one member variable named 'value'
	int value;
};

template<typename K, typename V>
class interval_map {
	friend void IntervalMapTest();
	V m_valBegin;
	std::map<K, V> m_map;
public:
	// constructor associates whole range of K with val
	interval_map(V const& val)
		: m_valBegin(val)
	{}

	// Assign value val to interval [keyBegin, keyEnd).
	// Overwrite previous values in this interval.
	// Conforming to the C++ Standard Library conventions, the interval
	// includes keyBegin, but excludes keyEnd.
	// If !( keyBegin < keyEnd ), this designates an empty interval,
	// and assign must do nothing.
	void assign(K const& keyBegin, K const& keyEnd, V const& val) {

		// this function could be rewritten in a much simpler form.
		// LinkedIn says programmer ususally attends 200 interviews until he gets a job. Imagine doing assignements for 800 hours

		// empty range
		if (!(keyBegin < keyEnd)) {
			return;
		}

		// special case when the new interval is fully enveloped in an already existing interval
		{
			auto prevToStartItr = m_map.lower_bound(keyBegin);
			if (prevToStartItr != m_map.end() && prevToStartItr != m_map.begin()) {
				--prevToStartItr;
			}
			auto prevToEndItr = m_map.lower_bound(keyEnd);
			if (prevToEndItr != m_map.end() && prevToEndItr != m_map.begin()) {
				--prevToEndItr;
			}
			if (prevToStartItr != m_map.end() && prevToStartItr == prevToEndItr) {
				// nothing to do, this range would be fully contained
				if (prevToStartItr->second == val) {
					return;
				}
			}
		}

		bool bSkipAddEnd = false;
		V valBeforeNewEnd = m_valBegin;
		// check if we are cutting in half some range
		// messed up check because we can't check for equal sign
		{
			auto endItr = m_map.find(keyEnd);
			if (endItr != m_map.end()) {
				// at our end, a new section is starting that would have the same value
				if (endItr->second == val) {
					bSkipAddEnd = true;
					m_map.erase(endItr);
				}
				else {
					valBeforeNewEnd = endItr->second;
				}
			}
			else {
				auto prevToEndItr = m_map.lower_bound(keyEnd);

				if (prevToEndItr != m_map.end() && prevToEndItr != m_map.begin()) {
					prevToEndItr--;
					valBeforeNewEnd = prevToEndItr->second;
				}
				else if ((prevToEndItr == m_map.end() || prevToEndItr == m_map.begin()) && val == m_valBegin) {
					bSkipAddEnd = true;
				}
			}
		}
		
		// special case when we are deleting the first entry
		bool bSkipAddFirst = false;
		if (val == m_valBegin) {
			auto isFirstKey = m_map.lower_bound(keyBegin);
			if (isFirstKey != m_map.end()) {
				if (keyBegin < isFirstKey->first) {
					--isFirstKey;
				}
				if (isFirstKey == m_map.begin() && !(isFirstKey->first < keyBegin) ) {
					bSkipAddFirst = true;
					if (val == m_valBegin) {
						m_map.erase(isFirstKey);
					}
				}
				else if (isFirstKey == m_map.end() && val == m_valBegin) {
					bSkipAddFirst = true;
				}
			}
			else if(val == m_valBegin) {
				bSkipAddFirst = true;
			}
		}

		// set the new range
		// !! it is not defined what happens if keybegin == keyend !!!!
		if (bSkipAddFirst == false) {
			m_map.insert_or_assign(keyBegin, val);
		}
		if (bSkipAddEnd == false) {
			m_map.insert_or_assign(keyEnd, valBeforeNewEnd);
		}

		// cleanup. Delete everything between the start and the end
		auto startItr = m_map.find(keyBegin);
		// keyBegin value did not get added
		if (startItr == m_map.end()) {
//			if( bSkipAddEnd == false 
			startItr = m_map.begin();
		}
		else if (startItr != m_map.end()) {
			startItr = std::next(startItr);
		}
		if (startItr != m_map.end() && keyBegin < startItr->first) {
			auto endItr = m_map.find(keyEnd);
			if ((startItr != m_map.end() && endItr == m_map.end() && startItr->first < keyEnd) ||
				(endItr != m_map.end() && startItr->first < endItr->first)) {
				m_map.erase(startItr, endItr);
			}
		}

		// if val is erasing, make sure to erase next / prev also
		// could have moved this code inside the add code. left it separated for the sake of readability
		{
			auto itr = m_map.find(keyEnd);
			if (itr != m_map.end())	{
				auto itr_next = std::next(itr);
				while (itr_next != m_map.end() && itr->second == itr_next->second) {
					m_map.erase(itr);
					itr = itr_next;
					itr_next = std::next(itr);
				}
			}
		}

		{
			auto itr = m_map.find(keyBegin);
			if (itr != m_map.end()) {
				auto itr_prev = std::prev(itr);
				while (itr_prev != m_map.end() && itr->second == itr_prev->second) {
					m_map.erase(itr);
					itr = itr_prev;
					itr_prev = std::prev(itr);
				}
			}
		}
	}

	void assign_unfinished_cleanup(K const& keyBegin, K const& keyEnd, V const& val) {

		// this function could be rewritten in a much simpler form.
		// LinkedIn says programmer ususally attends 200 interviews until he gets a job. Imagine doing assignements for 800 hours

		// empty range
		if (!(keyBegin < keyEnd)) {
			return;
		}

		auto preBegin = m_map.upper_bound(keyBegin); // go after begin
		auto postBegin = preBegin; // go after begin

		auto preEnd = m_map.upper_bound(keyEnd); // go after end
		auto postEnd = preEnd; // go after end

		if (preBegin != m_map.begin() && preBegin != m_map.end()) {
			preBegin--;
		}

		// not a valid itr. Might not happen for your compiler
		if (preBegin != m_map.end() && keyBegin < preBegin->first) {
			preBegin = m_map.end();
		}

		// not a valid itr. Might not happen for your compiler
		if (preEnd != m_map.end() && keyEnd < preEnd->first) {
			preEnd--;
		}

		bool bSkipAddFirst = false;
		if (!((val == m_valBegin && preBegin == m_map.end()) || (preBegin != m_map.end() && preBegin->second == val))) {
			m_map.insert_or_assign(keyBegin, val);
		}
		else {
			bSkipAddFirst = true;
		}

		// there is no next value, we become the last value
		if (postEnd == m_map.end()) {
			m_map.insert_or_assign(keyEnd, m_valBegin);
		}
		else if (preEnd != m_map.end()) {
			// cutting in half an interval 
			m_map.insert_or_assign(keyEnd, preEnd->second);
		}
		else {
			// we are not cutting half an interval
			m_map.insert_or_assign(keyEnd, m_valBegin);
		}

		// delete all values between new begin and new end
		auto toDelItr = postBegin;
		if (toDelItr != m_map.end() && keyBegin < toDelItr->first) {
			while (toDelItr != m_map.end() && toDelItr->first < keyEnd) {
				auto toDelItr2 = toDelItr;
				toDelItr++;
				m_map.erase(toDelItr2);
			}
		}

		// our end value is same as next end value
		toDelItr = preBegin;
		if (toDelItr != m_map.end())
		{
			auto toDelNextItr = toDelItr++;
			while (toDelNextItr != m_map.end() && toDelItr->second == toDelNextItr->second) {
				m_map.erase(toDelItr);
				toDelItr = toDelNextItr;
				toDelNextItr++;
			}
		}

		toDelItr = preEnd;
		if (toDelItr != m_map.end())
		{
			auto toDelNextItr = toDelItr++;
			while (toDelNextItr != m_map.end() && toDelItr->second == toDelNextItr->second) {
				m_map.erase(toDelItr);
				toDelItr = toDelNextItr;
				toDelNextItr++;
			}
		}
	}

	// look-up of the value associated with key
	V const& operator[](K const& key) const {
		auto it = m_map.upper_bound(key);
		if (it == m_map.begin() 
			|| it == m_map.end()) {
			return m_valBegin;
		}
		else {
			return (--it)->second;
		}
	}

	void printMapContent() {
		static int dTestCounter = 0;
		std::cout << dTestCounter << ") Everywhere value : " << m_valBegin << '\n';
		dTestCounter++;

		for (auto&& [key, val] : m_map) {
			std::cout << key << ':' << val << ',';
		}
		std::cout << '\n';
	}

	void checkExpected(std::string sExpectedContent) {
		static int dTestCounter = 0;
		dTestCounter++;
		std::string sContentNow;
		for (auto&& [key, val] : m_map) {
			sContentNow = sContentNow + key.toStr() + ":" + val.toStr() + ",";
		}
		if (sContentNow != sExpectedContent) {
			std::cout << "Test case failed " << dTestCounter << " Expected : " << sExpectedContent << " got : " << sContentNow << '\n';
		}
	}
};

int main()
{
	// had a headache .. missing cleanup and for some reason it's not functioning as expected
	interval_map<KeyK, ValueTypeV> M2(1);
	M2.assign(2, 5, 2); // test simple add range
	M2.checkExpected("2:2,5:1,");

	M2.assign(-1, 0, 3); // test add range before range
	M2.checkExpected("-1:3,0:1,2:2,5:1,");

	M2.assign(0, 2, 4); // test add range between range
	M2.checkExpected("-1:3,0:4,2:2,5:1,");

	M2.assign(0, 2, 2); // add range with overwrite
	M2.checkExpected("-1:3,0:2,5:1,");

	M2.assign(0, 2, 4);
	M2.checkExpected("-1:3,0:4,2:2,5:1,");

	M2.assign(1, 2, 1);
	M2.checkExpected("-1:3,0:4,1:1,2:2,5:1,");

	M2.assign(0, 3, 1);
	M2.checkExpected("-1:3,0:1,3:2,5:1,");

	M2.assign(-1, 5, 1);
	M2.checkExpected("");

	M2.assign(2, 5, 2);
	M2.checkExpected("2:2,5:1,");
	M2.assign(-1, 2, 3);
	M2.checkExpected("-1:3,2:2,5:1,");
	M2.assign(-1, 5, 4);
	M2.checkExpected("-1:4,5:1,");
	M2.assign(-1, 5, 1);
	M2.checkExpected("");

	M2.assign(2, 5, 2);
	M2.checkExpected("2:2,5:1,");
	M2.assign(5, 6, 3);
	M2.checkExpected("2:2,5:3,6:1,");


	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(2, 5, 2);
	M2.checkExpected("2:2,5:1,");
	M2.assign(7, 8, 1);
	M2.checkExpected("2:2,5:1,");

	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-5, 5, 2);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-4, 4, 1);
	M2.checkExpected("-5:2,-4:1,4:2,5:1,");
	M2.assign(-3, 3, 1);
	M2.checkExpected("-5:2,-4:1,4:2,5:1,");

	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-5, 5, 2);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-3, 3, 2);
	M2.checkExpected("-5:2,5:1,");

	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-5, 5, 2);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-9, -6, 1);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-9, -5, 1);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-9, -4, 1);
	M2.checkExpected("-4:2,5:1,");

	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-5, 5, 2);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-5, -4, 1);
	M2.checkExpected("-4:2,5:1,");
	M2.assign(-5, -4, 1);
	M2.checkExpected("-4:2,5:1,");
	M2.assign(-5, -4, 1);
	M2.checkExpected("-4:2,5:1,");
	M2.assign(-5, -2, 1);
	M2.checkExpected("-2:2,5:1,");

	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-5, 5, 2);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-7, -6, 3);
	M2.checkExpected("-7:3,-6:1,-5:2,5:1,");
	M2.assign(-7, -4, 4);
	M2.checkExpected("-7:4,-4:2,5:1,");

	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-5, 5, 2);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-7, -6, 3);
	M2.checkExpected("-7:3,-6:1,-5:2,5:1,");
	M2.assign(-7, 5, 4);
	M2.checkExpected("-7:4,5:1,");

	M2.assign(-10, 10, 1);
	M2.checkExpected("");
	M2.assign(-5, 5, 2);
	M2.checkExpected("-5:2,5:1,");
	M2.assign(-7, -6, 3);
	M2.checkExpected("-7:3,-6:1,-5:2,5:1,");
	M2.assign(-7, 6, 4);
	M2.checkExpected("-7:4,6:1,");

	return 0;
}