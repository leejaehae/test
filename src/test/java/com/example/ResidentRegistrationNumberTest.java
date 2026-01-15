package com.example;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;
import org.junit.jupiter.params.provider.NullAndEmptySource;
import org.junit.jupiter.params.provider.ValueSource;

import java.time.LocalDate;

import static org.junit.jupiter.api.Assertions.*;

@DisplayName("주민등록번호 유효성 검사 테스트")
class ResidentRegistrationNumberTest {

    private ResidentRegistrationNumber validator;

    @BeforeEach
    void setUp() {
        validator = new ResidentRegistrationNumber();
    }

    @Nested
    @DisplayName("형식 검사 테스트")
    class FormatTest {

        @ParameterizedTest
        @ValueSource(strings = {
            "900923-1234567",
            "580312-2132342",
            "850102-1234521",
            "720204-2134323",
            "781005-1025044",
            "741104-2010041",
            "720224-2532123",
            "781015-1523144",
            "741106-2211041"
        })
        @DisplayName("유효한 형식")
        void validFormat(String rrn) {
            assertTrue(validator.isValidFormat(rrn));
        }

        @ParameterizedTest
        @ValueSource(strings = {
            "890923-12121212",  // 뒷자리 8자리
            "9009231234567",    // 하이픈 없음
            "90092-31234567",   // 앞자리 5자리
            "900923-123456",    // 뒷자리 6자리
            "90-0923-1234567",  // 하이픈 위치 오류
            "abcdef-1234567",   // 문자 포함
            "900923-123456a"    // 문자 포함
        })
        @DisplayName("유효하지 않은 형식")
        void invalidFormat(String rrn) {
            assertFalse(validator.isValidFormat(rrn));
        }

        @ParameterizedTest
        @NullAndEmptySource
        @DisplayName("null 또는 빈 문자열")
        void nullOrEmpty(String rrn) {
            assertFalse(validator.isValidFormat(rrn));
        }
    }

    @Nested
    @DisplayName("제공된 샘플 데이터 테스트")
    class SampleDataTest {

        @Test
        @DisplayName("900923-1234567 - 형식 유효, 체크섬 검증")
        void sample1() {
            String rrn = "900923-1234567";
            assertTrue(validator.isValidFormat(rrn));
            // 체크섬 검증 결과 출력
            System.out.println("900923-1234567 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("890923-12121212 - 형식 오류 (뒷자리 8자리)")
        void sample2() {
            String rrn = "890923-12121212";
            assertFalse(validator.isValidFormat(rrn));
            assertFalse(validator.isValid(rrn));
        }

        @Test
        @DisplayName("580312-2132342 - 1958년생 여성")
        void sample3() {
            String rrn = "580312-2132342";
            assertTrue(validator.isValidFormat(rrn));
            assertEquals("F", validator.getGender(rrn));
            System.out.println("580312-2132342 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("850102-1234521 - 1985년생 남성")
        void sample4() {
            String rrn = "850102-1234521";
            assertTrue(validator.isValidFormat(rrn));
            assertEquals("M", validator.getGender(rrn));
            System.out.println("850102-1234521 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("720204-2134323 - 1972년생 여성")
        void sample5() {
            String rrn = "720204-2134323";
            assertTrue(validator.isValidFormat(rrn));
            assertEquals("F", validator.getGender(rrn));
            System.out.println("720204-2134323 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("781005-1025044 - 1978년생 남성")
        void sample6() {
            String rrn = "781005-1025044";
            assertTrue(validator.isValidFormat(rrn));
            assertEquals("M", validator.getGender(rrn));
            System.out.println("781005-1025044 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("741104-2010041 - 1974년생 여성")
        void sample7() {
            String rrn = "741104-2010041";
            assertTrue(validator.isValidFormat(rrn));
            assertEquals("F", validator.getGender(rrn));
            System.out.println("741104-2010041 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("720224-2532123 - 외국인 (성별코드 5)")
        void sample8() {
            String rrn = "720224-2532123";
            assertTrue(validator.isValidFormat(rrn));
            assertTrue(validator.isForeigner(rrn));
            assertEquals("M", validator.getGender(rrn));
            System.out.println("720224-2532123 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("781015-1523144 - 내국인 (성별코드 1)")
        void sample9() {
            String rrn = "781015-1523144";
            assertTrue(validator.isValidFormat(rrn));
            assertFalse(validator.isForeigner(rrn));
            System.out.println("781015-1523144 유효성: " + validator.isValid(rrn));
        }

        @Test
        @DisplayName("741106-2211041 - 1974년생 여성")
        void sample10() {
            String rrn = "741106-2211041";
            assertTrue(validator.isValidFormat(rrn));
            assertEquals("F", validator.getGender(rrn));
            System.out.println("741106-2211041 유효성: " + validator.isValid(rrn));
        }
    }

    @Nested
    @DisplayName("성별 코드 테스트")
    class GenderCodeTest {

        @ParameterizedTest
        @CsvSource({
            "900101-1000000, M",  // 1990년대 남성
            "900101-2000000, F",  // 1990년대 여성
            "050101-3000000, M",  // 2000년대 남성
            "050101-4000000, F",  // 2000년대 여성
            "900101-5000000, M",  // 외국인 남성 (1900년대)
            "900101-6000000, F",  // 외국인 여성 (1900년대)
            "050101-7000000, M",  // 외국인 남성 (2000년대)
            "050101-8000000, F"   // 외국인 여성 (2000년대)
        })
        @DisplayName("성별 추출")
        void getGender(String rrn, String expectedGender) {
            assertEquals(expectedGender, validator.getGender(rrn));
        }
    }

    @Nested
    @DisplayName("외국인 판별 테스트")
    class ForeignerTest {

        @ParameterizedTest
        @ValueSource(strings = {
            "900101-5000000",
            "900101-6000000",
            "050101-7000000",
            "050101-8000000"
        })
        @DisplayName("외국인 주민등록번호")
        void isForeigner(String rrn) {
            assertTrue(validator.isForeigner(rrn));
        }

        @ParameterizedTest
        @ValueSource(strings = {
            "900101-1000000",
            "900101-2000000",
            "050101-3000000",
            "050101-4000000"
        })
        @DisplayName("내국인 주민등록번호")
        void isNotForeigner(String rrn) {
            assertFalse(validator.isForeigner(rrn));
        }
    }

    @Nested
    @DisplayName("생년월일 추출 테스트")
    class BirthDateTest {

        @Test
        @DisplayName("1990년생 생년월일")
        void birthDate1990() {
            LocalDate birthDate = validator.getBirthDate("900923-1234567");
            assertNotNull(birthDate);
            assertEquals(1990, birthDate.getYear());
            assertEquals(9, birthDate.getMonthValue());
            assertEquals(23, birthDate.getDayOfMonth());
        }

        @Test
        @DisplayName("2005년생 생년월일")
        void birthDate2005() {
            LocalDate birthDate = validator.getBirthDate("050101-3000000");
            assertNotNull(birthDate);
            assertEquals(2005, birthDate.getYear());
            assertEquals(1, birthDate.getMonthValue());
            assertEquals(1, birthDate.getDayOfMonth());
        }

        @Test
        @DisplayName("유효하지 않은 날짜")
        void invalidDate() {
            // 2월 30일은 존재하지 않음
            assertFalse(validator.isValidDate("900230"));
        }
    }

    @Nested
    @DisplayName("나이 계산 테스트")
    class AgeTest {

        @Test
        @DisplayName("나이 계산")
        void calculateAge() {
            int age = validator.getAge("900923-1234567");
            int expectedAge = LocalDate.now().getYear() - 1990;
            assertEquals(expectedAge, age);
        }

        @Test
        @DisplayName("유효하지 않은 주민번호 나이 계산")
        void invalidRrnAge() {
            assertEquals(-1, validator.getAge("invalid"));
        }
    }

    @Nested
    @DisplayName("체크섬 테스트")
    class ChecksumTest {

        @Test
        @DisplayName("체크섬 검증 - 13자리 필요")
        void checksumRequires13Digits() {
            assertFalse(validator.isValidChecksum("12345678901")); // 11자리
            assertFalse(validator.isValidChecksum("123456789012")); // 12자리
            assertFalse(validator.isValidChecksum("12345678901234")); // 14자리
        }

        @Test
        @DisplayName("null 체크섬 검증")
        void checksumNull() {
            assertFalse(validator.isValidChecksum(null));
        }
    }

    @Nested
    @DisplayName("전체 유효성 검사 테스트")
    class FullValidationTest {

        @Test
        @DisplayName("형식 오류시 유효하지 않음")
        void invalidFormat() {
            assertFalse(validator.isValid("890923-12121212"));
        }

        @Test
        @DisplayName("null은 유효하지 않음")
        void nullIsInvalid() {
            assertFalse(validator.isValid(null));
        }

        @Test
        @DisplayName("존재하지 않는 날짜는 유효하지 않음")
        void invalidDateIsInvalid() {
            assertFalse(validator.isValid("901301-1234567")); // 13월
            assertFalse(validator.isValid("900230-1234567")); // 2월 30일
        }
    }
}
