#include "string.hpp"
#include <gtest/gtest.h>

#include <random>

TEST(Constructors, Default) {
  String s;
  ASSERT_EQ(s.Size(), 0);
  ASSERT_TRUE(s.Empty());
}

TEST(Constructors, FromNumberAndLetter) {
  const size_t size = 100;
  String s(size, 'a');
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s[i], 'a');
  }
}

TEST(Constructors, FromCString) {
  const char* cstr = "aaa";
  String s(cstr);
  for (size_t i = 0; i < 3; ++i) {
    ASSERT_EQ(s[i], 'a');
  }
}

TEST(Constructors, CopyConstructor) {
  const size_t size = 100;
  String s(size, 'a');
  String s1(s);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s[i], s1[i]);
  }
  ASSERT_NE(s.Data(), s1.Data());
}

TEST(Assignment, Simple) {
  const size_t size = 100;
  String s(size, 'a');
  String s1 = s;
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s[i], s1[i]);
  }
  ASSERT_NE(s.Data(), s1.Data());
}

TEST(Assignment, Self) {
  const size_t size = 100;
  String s(size, 'a');
  s = s;
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s[i], s[i]);
  }
  ASSERT_EQ(s.Data(), s.Data());
}

TEST(Clear, Clear) {
  const size_t size = 100;
  String s(size, 'a');
  s.Clear();
  ASSERT_EQ(s.Size(), 0);
  ASSERT_TRUE(s.Empty());
}

TEST(PushBack, ToExisting) {
  String s = "aba";
  s.PushBack('c');
  ASSERT_EQ(s.Size(), 4);
}

TEST(PushBack, ToDefault) {
  String s;
  s.PushBack('c');
  ASSERT_EQ(s.Size(), 1);
}

TEST(PushBack, FromNonEmpty) {
  String s = "ab";
  s.PopBack();
  ASSERT_EQ(s.Size(), 1);
  s.PopBack();
  ASSERT_EQ(s.Size(), 0);
}

TEST(PushBack, FromEmpty) {
  String s;
  s.PopBack();
  ASSERT_EQ(s.Size(), 0);
}

TEST(Resize, ToLess) {
  String s = "abacaba";
  s.Resize(3);
  ASSERT_EQ(s.Size(), 3);
  ASSERT_EQ(s[2], 'a');
}

TEST(Resize, ToBigger) {
  String s = "aba";
  s.Resize(10);
  ASSERT_EQ(s.Size(), 10);
}

TEST(Resize, ToZero) {
  String s = "aba";
  s.Resize(0);
  EXPECT_TRUE(s.Empty());
}

TEST(Resize, ToBiggerWithVal) {
  String s = "aba";
  s.Resize(10, 'a');
  ASSERT_EQ(s.Size(), 10);
  ASSERT_EQ(s[4], 'a');
  ASSERT_EQ(s[0], 'a');
  ASSERT_EQ(s[9], 'a');
}

TEST(Reserve, ToLess) {
  String s = "abacabababacabaabacaba";
  s.Reserve(1);
  ASSERT_EQ(s.Size(), 22);
  s.ShrinkToFit();
  ASSERT_EQ(s.Capacity(), 22);
}

TEST(Reserve, ToBigger) {
  String s = "abacabababacabaabacaba";
  s.Reserve(30);
  ASSERT_EQ(s.Size(), 22);
  ASSERT_GT(s.Capacity(), 22);
  s.ShrinkToFit();
  ASSERT_EQ(s.Capacity(), 22);
}

TEST(ShrinkToFit, ShrinkToFit) {
  String s = "abacabababacabaabacaba";
  s.ShrinkToFit();
  ASSERT_EQ(s.Capacity(), 22);
}

TEST(Swap, Two) {
  String s = "aboba";
  String t = "abiba";
  s.Swap(t);
  ASSERT_EQ(s.Size(), t.Size());
  ASSERT_EQ(s.Capacity(), t.Capacity());
  ASSERT_EQ(s[2], 'i');
  ASSERT_EQ(t[2], 'o');
}

TEST(Swap, One) {
  String s = "aboba";
  s.Swap(s);
  ASSERT_EQ(s.Size(), s.Size());
  ASSERT_EQ(s.Capacity(), s.Capacity());
  ASSERT_EQ(s[2], 'o');
}

TEST(SquareBrackets, NonConst) {
  String s = "aboba";
  const bool is_ref = std::is_reference_v<decltype(s[0])>;
  EXPECT_TRUE(is_ref);
  s[0] = 'c';
  const bool is_char = std::is_same_v<std::remove_reference_t<decltype(s[0])>, char>;
  EXPECT_TRUE(is_char);
}

TEST(SquareBrackets, Const) {
  const String s = "aboba";
  const bool is_const_ref =
      std::is_reference_v<decltype(s[0])> && std::is_const_v<std::remove_reference_t<decltype(s[0])>>;
  const bool is_not_ref = !std::is_reference_v<decltype(s[0])>;
  EXPECT_TRUE(is_const_ref || is_not_ref);
}

TEST(FrontBack, Const) {
  const String s = "abob";
  bool is_const_ref =
      std::is_reference_v<decltype(s.Front())> && std::is_const_v<std::remove_reference_t<decltype(s.Front())>>;
  bool is_not_ref = !std::is_reference_v<decltype(s.Front())>;
  EXPECT_TRUE(is_const_ref || is_not_ref);
  EXPECT_EQ(s.Front(), 'a');
  is_const_ref =
      std::is_reference_v<decltype(s.Back())> && std::is_const_v<std::remove_reference_t<decltype(s.Back())>>;
  is_not_ref = !std::is_reference_v<decltype(s.Back())>;
  EXPECT_TRUE(is_const_ref || is_not_ref);
  EXPECT_EQ(s.Back(), 'b');
}

TEST(FrontBack, NonConst) {
  String s = "abob";
  bool is_ref = std::is_reference_v<decltype(s.Front())>;
  bool is_char = std::is_same_v<std::remove_reference_t<decltype(s.Front())>, char>;
  EXPECT_TRUE(is_ref && is_char);
  s.Front() = 'c';
  EXPECT_EQ(s[0], 'c');

  is_ref = std::is_reference_v<decltype(s.Back())>;
  is_char = std::is_same_v<std::remove_reference_t<decltype(s.Back())>, char>;
  EXPECT_TRUE(is_ref && is_char);
  s.Back() = 'c';
  EXPECT_EQ(s[s.Size() - 1], 'c');
}

TEST(Data, NonConst) {
  String s = "abob";
  auto p = s.Data();
  bool are_same = std::is_same_v<decltype(s.Data()), const char*>;
  EXPECT_TRUE(are_same);
}

TEST(Data, Const) {
  const String s = "abob";
  bool are_same = std::is_same_v<decltype(s.Data()), const char*>;
  EXPECT_TRUE(are_same);
}

TEST(SizeCapacity, IsUnsigned) {
  String s;
  EXPECT_TRUE(std::is_unsigned_v<decltype(s.Size())>);
  EXPECT_TRUE(std::is_unsigned_v<decltype(s.Capacity())>);
}

TEST(Comparison, SameLen) {
  String s = "aboba";
  String t = "abiba";
  EXPECT_TRUE(t < s);
  EXPECT_FALSE(t > s);
  EXPECT_FALSE(t >= s);
  EXPECT_TRUE(t <= s);
  EXPECT_FALSE(t == s);
  EXPECT_TRUE(t != s);
}

TEST(Comparison, Const) {
  const String s = "aboba";
  const String t = "abiba";
  EXPECT_TRUE(t < s);
  EXPECT_FALSE(t > s);
  EXPECT_FALSE(t >= s);
  EXPECT_TRUE(t <= s);
  EXPECT_FALSE(t == s);
  EXPECT_TRUE(t != s);
}

TEST(Comparison, DiffLen) {
  String s = "abobabiba";
  String t = "aboba";
  EXPECT_TRUE(t < s);
  EXPECT_FALSE(t > s);
  EXPECT_FALSE(t >= s);
  EXPECT_TRUE(t <= s);
  EXPECT_FALSE(t == s);
  EXPECT_TRUE(t != s);
}

TEST(Comparison, Empty) {
  String s = "aboba";
  String t = "";
  EXPECT_TRUE(t < s);
  EXPECT_FALSE(t > s);
  EXPECT_FALSE(t >= s);
  EXPECT_TRUE(t <= s);
  EXPECT_FALSE(t == s);
  EXPECT_TRUE(t != s);
}

TEST(Iostream, In) {
  std::stringstream is{"olololo"};
  String s;
  is >> s;
  ASSERT_EQ(s, String("olololo"));
}

TEST(Iostream, Out) {
  std::stringstream os;
  String s = "lol";
  os << s;
  ASSERT_EQ(os.str(), "lol");
}

TEST(Iostream, ConstOut) {
  std::stringstream os;
  const String s = "lol";
  os << s;
  ASSERT_EQ(os.str(), "lol");
}

TEST(Concat, EasyPlus) {
  String s = "aboba";
  String t = "biba";
  EXPECT_TRUE(s + t == "abobabiba");
}

TEST(Concat, ConstPlus) {
  const String s = "aboba";
  const String t = "biba";
  EXPECT_TRUE(s + t == "abobabiba");
}

TEST(Concat, StressPlus) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 25);
  std::string expected;
  String s;
  const size_t num_iterations = 1 << 10;
  for (size_t i = 0; i < num_iterations; ++i) {
    String to_add = "a";
    to_add[0] += distrib(gen);
    expected.append(to_add.Data());
    s = s + to_add;
  }
  EXPECT_TRUE(s == expected.data());
}

TEST(Concat, PlusAssignmentCombo) {
  String s = "(";
  String t = ")";
  s += t + s += s + t += t;
  EXPECT_TRUE((s += t) == "()(()))");
  (s += t) = "f";
  EXPECT_TRUE(s == "f");
}

TEST(Concat, PlusAssignmentFibonacciStyle) {
  String s = "(";
  String t = ")";
  std::string s_s = "(";
  std::string t_s = ")";
  const size_t num_iterations = 18;
  for (size_t i = 0; i < num_iterations; ++i) {
    s += t;
    t += s;
    s_s += t_s;
    t_s += s_s;
  }
  EXPECT_TRUE(t == t_s.data());
  EXPECT_TRUE(s == s_s.data());
}

TEST(Multiply, Easy) {
  String s = "aba";
  EXPECT_TRUE(s * 2 == "abaaba");
}

TEST(Multiply, Const) {
  const String s = "aba";
  EXPECT_TRUE(s * 2 == "abaaba");
}

TEST(Multiply, Stress) {
  String s = "a";
  EXPECT_TRUE(s * 1000000 == String(1000000, 'a'));
}

TEST(Multiply, Assignment) {
  String s = "a";
  EXPECT_TRUE((s *= 1000000) == String(1000000, 'a'));
}

TEST(Multiply, ComboWithPlus) {
  String s = "a";
  String t = "b";
  s = (s += (s *= 2) + (t += s * 3) * 2) * 0;
  EXPECT_TRUE(s.Empty());
}

TEST(Split, Easy) {
  {
    std::vector<String> expected{"aba", "caba", "1"};
    EXPECT_TRUE(expected == String("aba caba 1").Split());
  }
  {
    std::vector<String> expected{"aba"};
    EXPECT_TRUE(expected == String("aba").Split());
  }
}

TEST(Split, Empty) {
  std::vector<String> expected{""};
  EXPECT_TRUE(expected == String("").Split());
}

TEST(Split, FullMatch) {
  std::vector<String> expected{"", ""};
  EXPECT_TRUE(expected == String("full match").Split("full match"));
}

TEST(Split, SomeTests) {
  {
    std::vector<String> expected{"just", "", "a", "test", ""};
    EXPECT_TRUE(expected == String("just  a test ").Split());
  }
  {
    std::vector<String> expected{"hello", "world,no split here", "", "1", ""};
    EXPECT_TRUE(expected == String("hello, world,no split here, , 1, ").Split(", "));
  }
  {
    std::vector<String> expected{"", "a", "b c", "def", "g h "};
    EXPECT_TRUE(expected == String("  a  b c  def  g h ").Split("  "));
  }
}

TEST(Join, Easy) {
  EXPECT_TRUE(String("aba") == String("b").Join({"a", "a"}));
}

TEST(Join, Empty) {
  EXPECT_TRUE(String("") == String("b").Join({}));
  EXPECT_TRUE(String("") == String("").Join({""}));
  EXPECT_TRUE(String(" ") == String("").Join({" "}));
}

TEST(Join, Stress) {
  String a = String("a") * 133700;
  String b = String("b") * 198400;
  String c = String("c") * 22800;
  String expected = c + a + b + a + c;
  EXPECT_TRUE(expected == b.Join({a, a}).Join({c, c}));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}