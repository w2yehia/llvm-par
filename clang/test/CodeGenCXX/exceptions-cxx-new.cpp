// RUN: %clang_cc1 %s -triple i686-pc-win32 -fms-extensions -fexceptions -fcxx-exceptions -fnew-ms-eh -emit-llvm -o - -std=c++11 | FileCheck %s

int f(int);

void test_catch() {
  try {
    f(1);
  } catch (int) {
    f(2);
  } catch (double) {
    f(3);
  }
}

// CHECK-LABEL: define void @"\01?test_catch@@YAXXZ"(
// CHECK:   invoke i32 @"\01?f@@YAHH@Z"(i32 1)
// CHECK:         to label %[[NORMAL:.*]] unwind label %[[CATCH_INT:.*]]

// CHECK: [[CATCH_INT]]
// CHECK:   %[[CATCHPAD_INT:.*]] = catchpad [{{.*}} @llvm.eh.handlertype.H.0, i8* null]
// CHECK:         to label %[[CATCH_INT_HANDLER:.*]] unwind label %[[CATCH_DOUBLE:.*]]

// CHECK: [[CATCH_INT_HANDLER]]
// CHECK:   invoke i32 @"\01?f@@YAHH@Z"(i32 2)
// CHECK:           to label %[[CATCH_INT_DONE:.*]] unwind label %[[CATCHENDPAD:.*]]

// CHECK: [[CATCH_INT_DONE]]
// CHECK:   catchret %[[CATCHPAD_INT]] to label %[[LEAVE_INT_CATCH:.*]]

// CHECK: [[LEAVE_INT_CATCH]]
// CHECK:   br label %[[LEAVE_FUNC:.*]]

// CHECK: [[LEAVE_FUNC]]
// CHECK:   ret void

// CHECK: [[CATCH_DOUBLE]]
// CHECK:   %[[CATCHPAD_DOUBLE:.*]] = catchpad [{{.*}} @llvm.eh.handlertype.N.0, i8* null]
// CHECK:           to label %[[CATCH_DOUBLE_HANDLER:.*]] unwind label %[[CATCHENDPAD]]

// CHECK: [[CATCH_DOUBLE_HANDLER]]
// CHECK:   invoke i32 @"\01?f@@YAHH@Z"(i32 3)
// CHECK:           to label %[[CATCH_DOUBLE_DONE:.*]] unwind label %[[CATCHENDPAD]]

// CHECK: [[CATCH_DOUBLE_DONE]]
// CHECK:   catchret %[[CATCHPAD_DOUBLE]] to label %[[LEAVE_DOUBLE_CATCH:.*]]

// CHECK: [[LEAVE_DOUBLE_CATCH]]
// CHECK:   br label %[[LEAVE_FUNC]]

// CHECK: [[CATCHENDPAD]]
// CHECK:   catchendpad unwind to caller

// CHECK: [[NORMAL]]
// CHECK:   br label %[[LEAVE_FUNC]]

struct Cleanup {
  ~Cleanup() { f(-1); }
};

void test_cleanup() {
  Cleanup C;
  f(1);
}

// CHECK-LABEL: define {{.*}} @"\01?test_cleanup@@YAXXZ"(
// CHECK:   invoke i32 @"\01?f@@YAHH@Z"(i32 1)
// CHECK:           to label %[[LEAVE_FUNC:.*]] unwind label %[[CLEANUP:.*]]

// CHECK: [[LEAVE_FUNC]]
// CHECK:   call x86_thiscallcc void @"\01??_DCleanup@@QAE@XZ"(
// CHECK:   ret void

// CHECK: [[CLEANUP]]
// CHECK:   %[[CLEANUPPAD:.*]] = cleanuppad []
// CHECK:   call x86_thiscallcc void @"\01??_DCleanup@@QAE@XZ"(
// CHECK:   cleanupret %[[CLEANUPPAD]] unwind to caller


// CHECK-LABEL: define {{.*}} void @"\01??1Cleanup@@QAE@XZ"(
// CHECK: entry:
// CHECK:   invoke i32 @"\01?f@@YAHH@Z"(i32 -1)
// CHECK:           to label %[[LEAVE_FUNC:.*]] unwind label %[[TERMINATE:.*]]

// CHECK: [[LEAVE_FUNC]]
// CHECK:   ret void

// CHECK: [[TERMINATE]]
// CHECK:   terminatepad [void ()* @"\01?terminate@@YAXXZ"] unwind to caller

