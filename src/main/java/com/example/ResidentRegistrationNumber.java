package com.example;

import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;

/**
 * 주민등록번호 유효성 검사 클래스
 */
public class ResidentRegistrationNumber {

    private static final int[] WEIGHTS = {2, 3, 4, 5, 6, 7, 8, 9, 2, 3, 4, 5};

    /**
     * 주민등록번호 전체 유효성 검사
     */
    public boolean isValid(String rrn) {
        if (!isValidFormat(rrn)) {
            return false;
        }

        String numbers = rrn.replace("-", "");

        if (!isValidDate(numbers)) {
            return false;
        }

        if (!isValidGenderCode(numbers)) {
            return false;
        }

        return isValidChecksum(numbers);
    }

    /**
     * 형식 검사 (6자리-7자리)
     */
    public boolean isValidFormat(String rrn) {
        if (rrn == null) {
            return false;
        }
        return rrn.matches("^\\d{6}-\\d{7}$");
    }

    /**
     * 생년월일 유효성 검사
     */
    public boolean isValidDate(String numbers) {
        if (numbers == null || numbers.length() < 7) {
            return false;
        }

        String datePart = numbers.substring(0, 6);
        char genderCode = numbers.charAt(6);

        int year = Integer.parseInt(datePart.substring(0, 2));
        int month = Integer.parseInt(datePart.substring(2, 4));
        int day = Integer.parseInt(datePart.substring(4, 6));

        // 성별 코드로 세기 결정
        int century = getCentury(genderCode);
        if (century == -1) {
            return false;
        }

        int fullYear = century + year;

        try {
            LocalDate date = LocalDate.of(fullYear, month, day);
            // 미래 날짜 체크
            return !date.isAfter(LocalDate.now());
        } catch (Exception e) {
            return false;
        }
    }

    /**
     * 성별 코드 유효성 검사
     */
    public boolean isValidGenderCode(String numbers) {
        if (numbers == null || numbers.length() < 7) {
            return false;
        }
        char genderCode = numbers.charAt(6);
        return "1234567890".indexOf(genderCode) >= 0;
    }

    /**
     * 체크섬 검증
     */
    public boolean isValidChecksum(String numbers) {
        if (numbers == null || numbers.length() != 13) {
            return false;
        }

        int sum = 0;
        for (int i = 0; i < 12; i++) {
            sum += Character.getNumericValue(numbers.charAt(i)) * WEIGHTS[i];
        }

        int checkDigit = (11 - (sum % 11)) % 10;
        int lastDigit = Character.getNumericValue(numbers.charAt(12));

        return checkDigit == lastDigit;
    }

    /**
     * 성별 추출 (M: 남성, F: 여성)
     */
    public String getGender(String rrn) {
        if (!isValidFormat(rrn)) {
            return null;
        }
        char genderCode = rrn.charAt(7);
        return (genderCode == '1' || genderCode == '3' || genderCode == '5' ||
                genderCode == '7' || genderCode == '9') ? "M" : "F";
    }

    /**
     * 생년월일 추출
     */
    public LocalDate getBirthDate(String rrn) {
        if (!isValidFormat(rrn)) {
            return null;
        }

        String numbers = rrn.replace("-", "");
        String datePart = numbers.substring(0, 6);
        char genderCode = numbers.charAt(6);

        int year = Integer.parseInt(datePart.substring(0, 2));
        int month = Integer.parseInt(datePart.substring(2, 4));
        int day = Integer.parseInt(datePart.substring(4, 6));

        int century = getCentury(genderCode);
        if (century == -1) {
            return null;
        }

        try {
            return LocalDate.of(century + year, month, day);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * 나이 계산
     */
    public int getAge(String rrn) {
        LocalDate birthDate = getBirthDate(rrn);
        if (birthDate == null) {
            return -1;
        }
        return LocalDate.now().getYear() - birthDate.getYear();
    }

    /**
     * 내국인/외국인 구분
     */
    public boolean isForeigner(String rrn) {
        if (!isValidFormat(rrn)) {
            return false;
        }
        char genderCode = rrn.charAt(7);
        return genderCode >= '5' && genderCode <= '8';
    }

    /**
     * 성별 코드로 세기 결정
     */
    private int getCentury(char genderCode) {
        switch (genderCode) {
            case '9':
            case '0':
                return 1800;
            case '1':
            case '2':
            case '5':
            case '6':
                return 1900;
            case '3':
            case '4':
            case '7':
            case '8':
                return 2000;
            default:
                return -1;
        }
    }
}
