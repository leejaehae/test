package com.example;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import java.util.List;
import java.util.Optional;

import static org.junit.jupiter.api.Assertions.*;

@DisplayName("UserService 테스트")
class UserServiceTest {

    private UserService userService;

    @BeforeEach
    void setUp() {
        userService = new UserService();
    }

    @Nested
    @DisplayName("사용자 생성 테스트")
    class CreateUserTest {

        @Test
        @DisplayName("정상적인 사용자 생성")
        void createUserSuccess() {
            User user = userService.createUser("홍길동", "hong@example.com", 25);

            assertNotNull(user);
            assertNotNull(user.getId());
            assertEquals("홍길동", user.getName());
            assertEquals("hong@example.com", user.getEmail());
            assertEquals(25, user.getAge());
        }

        @Test
        @DisplayName("null 이름으로 생성 실패")
        void createUserWithNullName() {
            assertThrows(IllegalArgumentException.class,
                () -> userService.createUser(null, "test@example.com", 25));
        }

        @Test
        @DisplayName("빈 이름으로 생성 실패")
        void createUserWithEmptyName() {
            assertThrows(IllegalArgumentException.class,
                () -> userService.createUser("", "test@example.com", 25));
        }

        @Test
        @DisplayName("50자 초과 이름으로 생성 실패")
        void createUserWithLongName() {
            String longName = "a".repeat(51);
            assertThrows(IllegalArgumentException.class,
                () -> userService.createUser(longName, "test@example.com", 25));
        }

        @Test
        @DisplayName("유효하지 않은 이메일로 생성 실패")
        void createUserWithInvalidEmail() {
            assertThrows(IllegalArgumentException.class,
                () -> userService.createUser("홍길동", "invalid-email", 25));
        }

        @Test
        @DisplayName("음수 나이로 생성 실패")
        void createUserWithNegativeAge() {
            assertThrows(IllegalArgumentException.class,
                () -> userService.createUser("홍길동", "test@example.com", -1));
        }

        @Test
        @DisplayName("150 초과 나이로 생성 실패")
        void createUserWithOverAge() {
            assertThrows(IllegalArgumentException.class,
                () -> userService.createUser("홍길동", "test@example.com", 151));
        }

        @Test
        @DisplayName("중복 이메일로 생성 실패")
        void createUserWithDuplicateEmail() {
            userService.createUser("홍길동", "hong@example.com", 25);

            assertThrows(IllegalStateException.class,
                () -> userService.createUser("김철수", "hong@example.com", 30));
        }
    }

    @Nested
    @DisplayName("사용자 조회 테스트")
    class FindUserTest {

        @Test
        @DisplayName("ID로 사용자 조회")
        void findById() {
            User created = userService.createUser("홍길동", "hong@example.com", 25);

            Optional<User> found = userService.findById(created.getId());

            assertTrue(found.isPresent());
            assertEquals(created.getId(), found.get().getId());
        }

        @Test
        @DisplayName("존재하지 않는 ID 조회")
        void findByIdNotFound() {
            Optional<User> found = userService.findById(999L);

            assertFalse(found.isPresent());
        }

        @Test
        @DisplayName("이메일로 사용자 조회")
        void findByEmail() {
            userService.createUser("홍길동", "hong@example.com", 25);

            Optional<User> found = userService.findByEmail("hong@example.com");

            assertTrue(found.isPresent());
            assertEquals("홍길동", found.get().getName());
        }

        @Test
        @DisplayName("모든 사용자 조회")
        void findAll() {
            userService.createUser("홍길동", "hong@example.com", 25);
            userService.createUser("김철수", "kim@example.com", 30);
            userService.createUser("이영희", "lee@example.com", 28);

            List<User> users = userService.findAll();

            assertEquals(3, users.size());
        }

        @Test
        @DisplayName("나이 범위로 사용자 조회")
        void findByAgeRange() {
            userService.createUser("홍길동", "hong@example.com", 25);
            userService.createUser("김철수", "kim@example.com", 30);
            userService.createUser("이영희", "lee@example.com", 35);
            userService.createUser("박민수", "park@example.com", 40);

            List<User> users = userService.findByAgeRange(25, 35);

            assertEquals(3, users.size());
        }
    }

    @Nested
    @DisplayName("사용자 수정 테스트")
    class UpdateUserTest {

        @Test
        @DisplayName("정상적인 사용자 수정")
        void updateUserSuccess() {
            User created = userService.createUser("홍길동", "hong@example.com", 25);

            User updated = userService.updateUser(created.getId(), "홍길동수정", "hong2@example.com", 26);

            assertEquals("홍길동수정", updated.getName());
            assertEquals("hong2@example.com", updated.getEmail());
            assertEquals(26, updated.getAge());
        }

        @Test
        @DisplayName("존재하지 않는 사용자 수정 실패")
        void updateUserNotFound() {
            assertThrows(IllegalArgumentException.class,
                () -> userService.updateUser(999L, "홍길동", "hong@example.com", 25));
        }

        @Test
        @DisplayName("같은 이메일로 수정 가능")
        void updateUserWithSameEmail() {
            User created = userService.createUser("홍길동", "hong@example.com", 25);

            User updated = userService.updateUser(created.getId(), "홍길동수정", "hong@example.com", 26);

            assertEquals("hong@example.com", updated.getEmail());
        }

        @Test
        @DisplayName("다른 사용자의 이메일로 수정 실패")
        void updateUserWithDuplicateEmail() {
            User user1 = userService.createUser("홍길동", "hong@example.com", 25);
            userService.createUser("김철수", "kim@example.com", 30);

            assertThrows(IllegalStateException.class,
                () -> userService.updateUser(user1.getId(), "홍길동", "kim@example.com", 25));
        }
    }

    @Nested
    @DisplayName("사용자 삭제 테스트")
    class DeleteUserTest {

        @Test
        @DisplayName("정상적인 사용자 삭제")
        void deleteUserSuccess() {
            User created = userService.createUser("홍길동", "hong@example.com", 25);

            boolean result = userService.deleteUser(created.getId());

            assertTrue(result);
            assertFalse(userService.findById(created.getId()).isPresent());
        }

        @Test
        @DisplayName("존재하지 않는 사용자 삭제")
        void deleteUserNotFound() {
            boolean result = userService.deleteUser(999L);

            assertFalse(result);
        }
    }

    @Nested
    @DisplayName("통계 테스트")
    class StatisticsTest {

        @Test
        @DisplayName("사용자 수 조회")
        void countUsers() {
            assertEquals(0, userService.countUsers());

            userService.createUser("홍길동", "hong@example.com", 25);
            userService.createUser("김철수", "kim@example.com", 30);

            assertEquals(2, userService.countUsers());
        }

        @Test
        @DisplayName("평균 나이 조회")
        void getAverageAge() {
            userService.createUser("홍길동", "hong@example.com", 20);
            userService.createUser("김철수", "kim@example.com", 30);
            userService.createUser("이영희", "lee@example.com", 40);

            assertEquals(30.0, userService.getAverageAge(), 0.01);
        }

        @Test
        @DisplayName("사용자 없을 때 평균 나이는 0")
        void getAverageAgeEmpty() {
            assertEquals(0.0, userService.getAverageAge(), 0.01);
        }
    }
}
