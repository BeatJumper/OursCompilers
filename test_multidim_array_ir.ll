; ModuleID = './test_multidim_array.c'
source_filename = "./test_multidim_array.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@__const.main.complex2d = private unnamed_addr constant [2 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4]], align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @process2D([3 x i32]* noundef %0, i32 noundef %1) #0 {
  %3 = alloca [3 x i32]*, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store [3 x i32]* %0, [3 x i32]** %3, align 8
  store i32 %1, i32* %4, align 4
  store i32 0, i32* %5, align 4
  store i32 0, i32* %6, align 4
  br label %8

8:                                                ; preds = %29, %2
  %9 = load i32, i32* %6, align 4
  %10 = load i32, i32* %4, align 4
  %11 = icmp slt i32 %9, %10
  br i1 %11, label %12, label %32

12:                                               ; preds = %8
  store i32 0, i32* %7, align 4
  br label %13

13:                                               ; preds = %16, %12
  %14 = load i32, i32* %7, align 4
  %15 = icmp slt i32 %14, 3
  br i1 %15, label %16, label %29

16:                                               ; preds = %13
  %17 = load i32, i32* %5, align 4
  %18 = load [3 x i32]*, [3 x i32]** %3, align 8
  %19 = load i32, i32* %6, align 4
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [3 x i32], [3 x i32]* %18, i64 %20
  %22 = load i32, i32* %7, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [3 x i32], [3 x i32]* %21, i64 0, i64 %23
  %25 = load i32, i32* %24, align 4
  %26 = add nsw i32 %17, %25
  store i32 %26, i32* %5, align 4
  %27 = load i32, i32* %7, align 4
  %28 = add nsw i32 %27, 1
  store i32 %28, i32* %7, align 4
  br label %13, !llvm.loop !6

29:                                               ; preds = %13
  %30 = load i32, i32* %6, align 4
  %31 = add nsw i32 %30, 1
  store i32 %31, i32* %6, align 4
  br label %8, !llvm.loop !8

32:                                               ; preds = %8
  %33 = load i32, i32* %5, align 4
  ret i32 %33
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @process3D([2 x [2 x i32]]* noundef %0, i32 noundef %1) #0 {
  %3 = alloca [2 x [2 x i32]]*, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store [2 x [2 x i32]]* %0, [2 x [2 x i32]]** %3, align 8
  store i32 %1, i32* %4, align 4
  store i32 0, i32* %5, align 4
  store i32 0, i32* %6, align 4
  br label %9

9:                                                ; preds = %40, %2
  %10 = load i32, i32* %6, align 4
  %11 = load i32, i32* %4, align 4
  %12 = icmp slt i32 %10, %11
  br i1 %12, label %13, label %43

13:                                               ; preds = %9
  store i32 0, i32* %7, align 4
  br label %14

14:                                               ; preds = %37, %13
  %15 = load i32, i32* %7, align 4
  %16 = icmp slt i32 %15, 2
  br i1 %16, label %17, label %40

17:                                               ; preds = %14
  store i32 0, i32* %8, align 4
  br label %18

18:                                               ; preds = %21, %17
  %19 = load i32, i32* %8, align 4
  %20 = icmp slt i32 %19, 2
  br i1 %20, label %21, label %37

21:                                               ; preds = %18
  %22 = load i32, i32* %5, align 4
  %23 = load [2 x [2 x i32]]*, [2 x [2 x i32]]** %3, align 8
  %24 = load i32, i32* %6, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %23, i64 %25
  %27 = load i32, i32* %7, align 4
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %26, i64 0, i64 %28
  %30 = load i32, i32* %8, align 4
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds [2 x i32], [2 x i32]* %29, i64 0, i64 %31
  %33 = load i32, i32* %32, align 4
  %34 = add nsw i32 %22, %33
  store i32 %34, i32* %5, align 4
  %35 = load i32, i32* %8, align 4
  %36 = add nsw i32 %35, 1
  store i32 %36, i32* %8, align 4
  br label %18, !llvm.loop !9

37:                                               ; preds = %18
  %38 = load i32, i32* %7, align 4
  %39 = add nsw i32 %38, 1
  store i32 %39, i32* %7, align 4
  br label %14, !llvm.loop !10

40:                                               ; preds = %14
  %41 = load i32, i32* %6, align 4
  %42 = add nsw i32 %41, 1
  store i32 %42, i32* %6, align 4
  br label %9, !llvm.loop !11

43:                                               ; preds = %9
  %44 = load i32, i32* %5, align 4
  ret i32 %44
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca [2 x [3 x i32]], align 16
  %3 = alloca [3 x [2 x [2 x i32]]], align 16
  %4 = alloca [2 x [2 x i32]], align 16
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %8 = bitcast [2 x [3 x i32]]* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %8, i8 0, i64 24, i1 false)
  %9 = bitcast [3 x [2 x [2 x i32]]]* %3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %9, i8 0, i64 48, i1 false)
  %10 = bitcast [2 x [2 x i32]]* %4 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %10, i8* align 16 bitcast ([2 x [2 x i32]]* @__const.main.complex2d to i8*), i64 16, i1 false)
  %11 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %2, i64 0, i64 0
  %12 = getelementptr inbounds [3 x i32], [3 x i32]* %11, i64 0, i64 0
  store i32 1, i32* %12, align 16
  %13 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %2, i64 0, i64 0
  %14 = getelementptr inbounds [3 x i32], [3 x i32]* %13, i64 0, i64 1
  store i32 2, i32* %14, align 4
  %15 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %2, i64 0, i64 0
  %16 = getelementptr inbounds [3 x i32], [3 x i32]* %15, i64 0, i64 2
  store i32 3, i32* %16, align 8
  %17 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %2, i64 0, i64 1
  %18 = getelementptr inbounds [3 x i32], [3 x i32]* %17, i64 0, i64 0
  store i32 4, i32* %18, align 4
  %19 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %2, i64 0, i64 1
  %20 = getelementptr inbounds [3 x i32], [3 x i32]* %19, i64 0, i64 1
  store i32 5, i32* %20, align 4
  %21 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %2, i64 0, i64 1
  %22 = getelementptr inbounds [3 x i32], [3 x i32]* %21, i64 0, i64 2
  store i32 6, i32* %22, align 4
  %23 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
  %24 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %23, i64 0, i64 0
  %25 = getelementptr inbounds [2 x i32], [2 x i32]* %24, i64 0, i64 0
  store i32 10, i32* %25, align 16
  %26 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
  %27 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %26, i64 0, i64 0
  %28 = getelementptr inbounds [2 x i32], [2 x i32]* %27, i64 0, i64 1
  store i32 20, i32* %28, align 4
  %29 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
  %30 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %29, i64 0, i64 1
  %31 = getelementptr inbounds [2 x i32], [2 x i32]* %30, i64 0, i64 0
  store i32 30, i32* %31, align 8
  %32 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
  %33 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %32, i64 0, i64 1
  %34 = getelementptr inbounds [2 x i32], [2 x i32]* %33, i64 0, i64 1
  store i32 40, i32* %34, align 4
  %35 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
  %36 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %35, i64 0, i64 0
  %37 = getelementptr inbounds [2 x i32], [2 x i32]* %36, i64 0, i64 0
  store i32 50, i32* %37, align 16
  %38 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
  %39 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %38, i64 0, i64 0
  %40 = getelementptr inbounds [2 x i32], [2 x i32]* %39, i64 0, i64 1
  store i32 60, i32* %40, align 4
  %41 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
  %42 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %41, i64 0, i64 1
  %43 = getelementptr inbounds [2 x i32], [2 x i32]* %42, i64 0, i64 0
  store i32 70, i32* %43, align 8
  %44 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
  %45 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %44, i64 0, i64 1
  %46 = getelementptr inbounds [2 x i32], [2 x i32]* %45, i64 0, i64 1
  store i32 80, i32* %46, align 4
  %47 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
  %48 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %47, i64 0, i64 0
  %49 = getelementptr inbounds [2 x i32], [2 x i32]* %48, i64 0, i64 0
  store i32 90, i32* %49, align 16
  %50 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
  %51 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %50, i64 0, i64 0
  %52 = getelementptr inbounds [2 x i32], [2 x i32]* %51, i64 0, i64 1
  store i32 100, i32* %52, align 4
  %53 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
  %54 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %53, i64 0, i64 1
  %55 = getelementptr inbounds [2 x i32], [2 x i32]* %54, i64 0, i64 0
  store i32 110, i32* %55, align 8
  %56 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
  %57 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %56, i64 0, i64 1
  %58 = getelementptr inbounds [2 x i32], [2 x i32]* %57, i64 0, i64 1
  store i32 120, i32* %58, align 4
  %59 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %2, i64 0, i64 0
  %60 = call i32 @process2D([3 x i32]* noundef %59, i32 noundef 2)
  store i32 %60, i32* %5, align 4
  %61 = load i32, i32* %5, align 4
  %62 = call i32 (i32, ...) bitcast (i32 (...)* @putint to i32 (i32, ...)*)(i32 noundef %61)
  %63 = call i32 (i32, ...) bitcast (i32 (...)* @putch to i32 (i32, ...)*)(i32 noundef 10)
  %64 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
  %65 = call i32 @process3D([2 x [2 x i32]]* noundef %64, i32 noundef 3)
  store i32 %65, i32* %6, align 4
  %66 = load i32, i32* %6, align 4
  %67 = call i32 (i32, ...) bitcast (i32 (...)* @putint to i32 (i32, ...)*)(i32 noundef %66)
  %68 = call i32 (i32, ...) bitcast (i32 (...)* @putch to i32 (i32, ...)*)(i32 noundef 10)
  %69 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %4, i64 0, i64 0
  %70 = getelementptr inbounds [2 x i32], [2 x i32]* %69, i64 0, i64 0
  %71 = load i32, i32* %70, align 16
  %72 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %4, i64 0, i64 0
  %73 = getelementptr inbounds [2 x i32], [2 x i32]* %72, i64 0, i64 1
  %74 = load i32, i32* %73, align 4
  %75 = add nsw i32 %71, %74
  %76 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %4, i64 0, i64 1
  %77 = getelementptr inbounds [2 x i32], [2 x i32]* %76, i64 0, i64 0
  %78 = load i32, i32* %77, align 8
  %79 = add nsw i32 %75, %78
  %80 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %4, i64 0, i64 1
  %81 = getelementptr inbounds [2 x i32], [2 x i32]* %80, i64 0, i64 1
  %82 = load i32, i32* %81, align 4
  %83 = add nsw i32 %79, %82
  store i32 %83, i32* %7, align 4
  %84 = load i32, i32* %7, align 4
  %85 = call i32 (i32, ...) bitcast (i32 (...)* @putint to i32 (i32, ...)*)(i32 noundef %84)
  %86 = call i32 (i32, ...) bitcast (i32 (...)* @putch to i32 (i32, ...)*)(i32 noundef 10)
  ret i32 0
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

declare i32 @putint(...) #3

declare i32 @putch(...) #3

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nofree nounwind willreturn writeonly }
attributes #2 = { argmemonly nofree nounwind willreturn }
attributes #3 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 14.0.0-1ubuntu1.1"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}
