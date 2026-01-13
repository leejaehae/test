package com.example;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;
import org.junit.jupiter.params.provider.ValueSource;

import static org.junit.jupiter.api.Assertions.*;

@DisplayName("Calculator 테스트")
class CalculatorTest {

    private Calculator calculator;

    @BeforeEach
    void setUp() {
        calculator = new Calculator();
    }

    @Nested
    @DisplayName("덧셈 테스트")
    class AddTest {

        @Test
        @DisplayName("양수 + 양수")
        void addPositiveNumbers() {
            assertEquals(5, calculator.add(2, 3));
        }

        @Test
        @DisplayName("음수 + 음수")
        void addNegativeNumbers() {
            assertEquals(-5, calculator.add(-2, -3));
        }

        @Test
        @DisplayName("양수 + 음수")
        void addMixedNumbers() {
            assertEquals(-1, calculator.add(2, -3));
        }

        @Test
        @DisplayName("0 더하기")
        void addWithZero() {
            assertEquals(5, calculator.add(5, 0));
            assertEquals(5, calculator.add(0, 5));
        }
    }

    @Nested
    @DisplayName("뺄셈 테스트")
    class SubtractTest {

        @Test
        @DisplayName("양수 - 양수")
        void subtractPositiveNumbers() {
            assertEquals(2, calculator.subtract(5, 3));
        }

        @Test
        @DisplayName("음수 - 음수")
        void subtractNegativeNumbers() {
            assertEquals(1, calculator.subtract(-2, -3));
        }

        @Test
        @DisplayName("결과가 음수인 경우")
        void subtractResultNegative() {
            assertEquals(-3, calculator.subtract(2, 5));
        }
    }

    @Nested
    @DisplayName("곱셈 테스트")
    class MultiplyTest {

        @Test
        @DisplayName("양수 * 양수")
        void multiplyPositiveNumbers() {
            assertEquals(6, calculator.multiply(2, 3));
        }

        @Test
        @DisplayName("음수 * 음수 = 양수")
        void multiplyNegativeNumbers() {
            assertEquals(6, calculator.multiply(-2, -3));
        }

        @Test
        @DisplayName("0 곱하기")
        void multiplyWithZero() {
            assertEquals(0, calculator.multiply(5, 0));
            assertEquals(0, calculator.multiply(0, 5));
        }
    }

    @Nested
    @DisplayName("나눗셈 테스트")
    class DivideTest {

        @Test
        @DisplayName("정상 나눗셈")
        void divideNormally() {
            assertEquals(2, calculator.divide(6, 3));
        }

        @Test
        @DisplayName("0으로 나누기 예외 발생")
        void divideByZeroThrowsException() {
            IllegalArgumentException exception = assertThrows(
                IllegalArgumentException.class,
                () -> calculator.divide(5, 0)
            );
            assertEquals("0으로 나눌 수 없습니다.", exception.getMessage());
        }

        @Test
        @DisplayName("정수 나눗셈 (소수점 버림)")
        void divideIntegerDivision() {
            assertEquals(2, calculator.divide(7, 3));
        }
    }

    @Nested
    @DisplayName("짝수 판별 테스트")
    class IsEvenTest {

        @ParameterizedTest
        @ValueSource(ints = {0, 2, 4, 100, -2, -4})
        @DisplayName("짝수 판별")
        void isEvenReturnsTrue(int number) {
            assertTrue(calculator.isEven(number));
        }

        @ParameterizedTest
        @ValueSource(ints = {1, 3, 5, 101, -1, -3})
        @DisplayName("홀수 판별")
        void isEvenReturnsFalse(int number) {
            assertFalse(calculator.isEven(number));
        }
    }

    @Nested
    @DisplayName("소수 판별 테스트")
    class IsPrimeTest {

        @ParameterizedTest
        @ValueSource(ints = {2, 3, 5, 7, 11, 13, 17, 19, 23})
        @DisplayName("소수 판별")
        void isPrimeReturnsTrue(int number) {
            assertTrue(calculator.isPrime(number));
        }

        @ParameterizedTest
        @ValueSource(ints = {0, 1, 4, 6, 8, 9, 10, 12, 15})
        @DisplayName("소수가 아닌 수 판별")
        void isPrimeReturnsFalse(int number) {
            assertFalse(calculator.isPrime(number));
        }
    }

    @Nested
    @DisplayName("파라미터화 테스트 예제")
    class ParameterizedTests {

        @ParameterizedTest
        @CsvSource({
            "1, 1, 2",
            "2, 3, 5",
            "10, 20, 30",
            "-1, 1, 0",
            "0, 0, 0"
        })
        @DisplayName("덧셈 파라미터화 테스트")
        void addParameterized(int a, int b, int expected) {
            assertEquals(expected, calculator.add(a, b));
        }

        @ParameterizedTest
        @CsvSource({
            "2, 3, 6",
            "5, 4, 20",
            "-2, 3, -6",
            "0, 100, 0"
        })
        @DisplayName("곱셈 파라미터화 테스트")
        void multiplyParameterized(int a, int b, int expected) {
            assertEquals(expected, calculator.multiply(a, b));
        }
    }
}
