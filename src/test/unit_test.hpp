#ifndef UNIT_TEST_HPP
#define UNIT_TEST_HPP

//reference https://github.com/wanmuc/UnitTest

#include<iostream>
#include<vector>
#include<chrono>
#include<string>

#define GREEN        "\033[0;32;32m"
#define RED          "\033[0;32;31m"
#define NONE         "\033[m"

namespace coral {

class TestCase {
public:
	virtual void run() = 0;
	virtual void TestCaseRun() { run(); }

	bool result() const { return result_; }
	void setResult(bool res) { result_ = res; }

	std::string caseName() const { return caseName_; }

	TestCase(std::string name) : caseName_(name) {}
private:
	bool result_{ true };
	std::string caseName_;
};

class UnitTestCore {
public:
	static UnitTestCore& Instance() {
		static UnitTestCore instance;
		return instance;
	}

	int run() {
		std::cout << "[==========] Running " << testCases_.size()
			<< " test case." << NONE << std::endl;
		int64_t total = 0LL;
		for (auto testCase : testCases_) {
			auto t1 = std::chrono::steady_clock::now();
			std::cout << GREEN << "[ RUN      ] " << testCase->caseName() << NONE << std::endl;
			testCase->TestCaseRun();
			std::cout << GREEN << "[      END ] " << testCase->caseName() << NONE << std::endl;
			auto t2 = std::chrono::steady_clock::now();
			auto d = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
			total += d.count();
			if (testCase->result()) {
				std::cout << GREEN << "[  PASSED  ] " << testCase->caseName() << " ("
					<< d.count() << " ms)" << NONE << std::endl;
				successCount_++;
			}
			else {
				std::cout << RED << "[  FAILED  ] " << testCase->caseName() << "("
					<< d.count() << " ms)" << NONE << std::endl;
				failureCount_++;
			}
			std::cout << "[----------]" << std::endl;
		}

		std::cout << "[==========] Total TestCase: " << testCases_.size()
			<< " (" << total << " ms)" << NONE << std::endl;
		if (successCount_ == 1) {
			std::cout << GREEN << "[  PASSED  ] " << successCount_ << " test." << NONE << std::endl;
		}
		else if (successCount_ > 1) {
			std::cout << GREEN << "[  PASSED  ] " << successCount_ << " tests." << NONE << std::endl;
		}

		if (failureCount_ == 1) {
			std::cout << RED << "[  FAILED  ] " << failureCount_ << " test." << NONE << std::endl;
		}
		else if (failureCount_ > 1) {
			std::cout << RED << "[  FAILED  ] " << failureCount_ << " tests." << NONE << std::endl;
		}
		return 0;
	}

	TestCase* Register(TestCase* testCase) {
		testCases_.push_back(testCase);
		return testCase;
	}

private:
	bool result_{ true };
	int32_t successCount_{ 0 };
	int32_t failureCount_{ 0 };
	std::vector<TestCase*>testCases_;
};

#define TEST_CASE_CLASS(test_case_name)                                                      \
  class test_case_name : public coral::TestCase {                                        \
   public:                                                                                   \
    test_case_name(std::string case_name) : coral::TestCase(case_name) {}                \
    virtual void run();                                                                      \
                                                                                             \
   private:                                                                                  \
    static coral::TestCase *const test_case_;                                            \
  };                                                                                         \
  coral::TestCase *const test_case_name::test_case_ =                                    \
      coral::UnitTestCore::Instance().Register(new test_case_name(#test_case_name));     \
  void test_case_name::run()

#define TEST_CASE(test_case_name) TEST_CASE_CLASS(test_case_name)

#define ASSERT_EQ(left, right)                                                                                     \
  if ((left) != (right)) {                                                                                         \
    std::cout << RED << "assert_eq failed at " << __FILE__ << ":" << __LINE__ << ". " << (left)                    \
              << "!=" << (right) << NONE << std::endl;                                                             \
    setResult(false);                                                                                              \
    return;                                                                                                        \
  }

#define ASSERT_NE(left, right)                                                                                     \
  if ((left) == (right)) {                                                                                         \
    std::cout << RED << "assert_ne failed at " << __FILE__ << ":" << __LINE__ << ". " << (left)                    \
              << "==" << (right) << NONE << std::endl;                                                             \
    setResult(false);                                                                                              \
    return;                                                                                                        \
  }

#define ASSERT_LT(left, right)                                                                                     \
  if ((left) >= (right)) {                                                                                         \
    std::cout << RED << "assert_lt failed at " << __FILE__ << ":" << __LINE__ << ". " << (left)                    \
              << ">=" << (right) << NONE << std::endl;                                                             \
    setResult(false);                                                                                              \
    return;                                                                                                        \
  }

#define ASSERT_LE(left, right)                                                                                     \
  if ((left) > (right)) {                                                                                          \
    std::cout << RED << "assert_le failed at " << __FILE__ << ":" << __LINE__ << ". " << (left) << ">"             \
              << (right) << NONE << std::endl;                                                                     \
    setResult(false);                                                                                              \
    return;                                                                                                        \
  }

#define ASSERT_GT(left, right)                                                                                     \
  if ((left) <= (right)) {                                                                                         \
    std::cout << RED << "assert_gt failed at " << __FILE__ << ":" << __LINE__ << ". " << (left)                    \
              << "<=" << (right) << NONE << std::endl;                                                             \
    setResult(false);                                                                                              \
    return;                                                                                                        \
  }

#define ASSERT_GE(left, right)                                                                                     \
  if ((left) < (right)) {                                                                                          \
    std::cout << RED << "assert_ge failed at " << __FILE__ << ":" << __LINE__ << ". " << (left) << "<"             \
              << (right) << NONE << std::endl;                                                                     \
    setResult(false);                                                                                              \
    return;                                                                                                        \
  }

#define ASSERT_TRUE(expr)                                                                                          \
  if (not(expr)) {                                                                                                 \
    std::cout << RED << "assert_true failed at " << __FILE__ << ":" << __LINE__ << ". " << (expr)                  \
              << " is false" << NONE << std::endl;                                                                 \ 
	setResult(false);                                                                                              \
		return;                                                                                                        \
  }

#define ASSERT_FALSE(expr)                                                                                         \
  if ((expr)) {                                                                                                    \
    std::cout << RED << "assert_false failed at " << __FILE__ << ":" << __LINE__ << ". " << (expr)                 \
              << " if true" << NONE << std::endl;                                                                  \
    setResult(false);                                                                                              \
    return;                                                                                                        \
  }

#define RUN_ALL_TESTS() \
coral::UnitTestCore::Instance().run();

} //namespace coral

#endif //UNIT_TEST_HPP