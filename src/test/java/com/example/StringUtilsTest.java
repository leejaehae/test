package com.example;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;
import org.junit.jupiter.params.provider.NullAndEmptySource;
import org.junit.jupiter.params.provider.ValueSource;

import static org.junit.jupiter.api.Assertions.*;

@DisplayName("StringUtils 테스트")
class StringUtilsTest {

    private StringUtils stringUtils;

    @BeforeEach
    void setUp() {
        stringUtils = new StringUtils();
    }

    @Nested
    @DisplayName("isEmpty 테스트")
    class IsEmptyTest {

        @Test
        @DisplayName("null은 empty")
        void nullIsEmpty() {
            assertTrue(stringUtils.isEmpty(null));
        }

        @Test
        @DisplayName("빈 문자열은 empty")
        void emptyStringIsEmpty() {
            assertTrue(stringUtils.isEmpty(""));
        }

        @Test
        @DisplayName("공백만 있는 문자열은 empty가 아님")
        void whitespaceIsNotEmpty() {
            assertFalse(stringUtils.isEmpty("   "));
        }

        @Test
        @DisplayName("일반 문자열은 empty가 아님")
        void normalStringIsNotEmpty() {
            assertFalse(stringUtils.isEmpty("hello"));
        }
    }

    @Nested
    @DisplayName("isBlank 테스트")
    class IsBlankTest {

        @ParameterizedTest
        @NullAndEmptySource
        @ValueSource(strings = {"   ", "\t", "\n", "  \t\n  "})
        @DisplayName("null, 빈 문자열, 공백만 있는 문자열은 blank")
        void blankStrings(String input) {
            assertTrue(stringUtils.isBlank(input));
        }

        @ParameterizedTest
        @ValueSource(strings = {"a", " a ", "hello world"})
        @DisplayName("문자가 포함된 문자열은 blank가 아님")
        void nonBlankStrings(String input) {
            assertFalse(stringUtils.isBlank(input));
        }
    }

    @Nested
    @DisplayName("reverse 테스트")
    class ReverseTest {

        @Test
        @DisplayName("null 반환")
        void reverseNull() {
            assertNull(stringUtils.reverse(null));
        }

        @Test
        @DisplayName("빈 문자열 반환")
        void reverseEmpty() {
            assertEquals("", stringUtils.reverse(""));
        }

        @ParameterizedTest
        @CsvSource({
            "hello, olleh",
            "abc, cba",
            "12345, 54321",
            "a, a"
        })
        @DisplayName("문자열 뒤집기")
        void reverseString(String input, String expected) {
            assertEquals(expected, stringUtils.reverse(input));
        }
    }

    @Nested
    @DisplayName("isPalindrome 테스트")
    class IsPalindromeTest {

        @Test
        @DisplayName("null은 회문이 아님")
        void nullIsNotPalindrome() {
            assertFalse(stringUtils.isPalindrome(null));
        }

        @ParameterizedTest
        @ValueSource(strings = {"", "a", "aba", "abba", "A man a plan a canal Panama"})
        @DisplayName("회문 문자열")
        void palindromes(String input) {
            assertTrue(stringUtils.isPalindrome(input));
        }

        @ParameterizedTest
        @ValueSource(strings = {"abc", "hello", "ab"})
        @DisplayName("회문이 아닌 문자열")
        void notPalindromes(String input) {
            assertFalse(stringUtils.isPalindrome(input));
        }
    }

    @Nested
    @DisplayName("countOccurrences 테스트")
    class CountOccurrencesTest {

        @Test
        @DisplayName("null 문자열은 0 반환")
        void countInNull() {
            assertEquals(0, stringUtils.countOccurrences(null, 'a'));
        }

        @Test
        @DisplayName("빈 문자열은 0 반환")
        void countInEmpty() {
            assertEquals(0, stringUtils.countOccurrences("", 'a'));
        }

        @ParameterizedTest
        @CsvSource({
            "hello, l, 2",
            "aaaaaa, a, 6",
            "hello, z, 0",
            "Hello World, o, 2"
        })
        @DisplayName("문자 개수 세기")
        void countCharacters(String input, char target, int expected) {
            assertEquals(expected, stringUtils.countOccurrences(input, target));
        }
    }

    @Nested
    @DisplayName("capitalize 테스트")
    class CapitalizeTest {

        @Test
        @DisplayName("null 반환")
        void capitalizeNull() {
            assertNull(stringUtils.capitalize(null));
        }

        @Test
        @DisplayName("빈 문자열 반환")
        void capitalizeEmpty() {
            assertEquals("", stringUtils.capitalize(""));
        }

        @ParameterizedTest
        @CsvSource({
            "hello, Hello",
            "HELLO, Hello",
            "hELLO, Hello",
            "a, A"
        })
        @DisplayName("첫 글자 대문자화")
        void capitalizeStrings(String input, String expected) {
            assertEquals(expected, stringUtils.capitalize(input));
        }
    }

    @Nested
    @DisplayName("truncate 테스트")
    class TruncateTest {

        @Test
        @DisplayName("null 반환")
        void truncateNull() {
            assertNull(stringUtils.truncate(null, 5));
        }

        @Test
        @DisplayName("최대 길이보다 짧으면 그대로 반환")
        void noTruncateNeeded() {
            assertEquals("hello", stringUtils.truncate("hello", 10));
        }

        @Test
        @DisplayName("최대 길이와 같으면 그대로 반환")
        void exactLength() {
            assertEquals("hello", stringUtils.truncate("hello", 5));
        }

        @Test
        @DisplayName("최대 길이보다 길면 잘라서 ... 추가")
        void truncateWithEllipsis() {
            assertEquals("hel...", stringUtils.truncate("hello world", 3));
        }
    }
}
