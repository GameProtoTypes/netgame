; ModuleID = 'test.cl'
source_filename = "test.cl"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.33.31629"

; Function Attrs: convergent norecurse nounwind uwtable
define dso_local spir_kernel void @test(ptr addrspace(1) nocapture noundef readonly align 4 %0, ptr addrspace(1) nocapture noundef writeonly align 4 %1) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 {
  %3 = tail call i32 @get_global_id(i32 noundef 0) #3
  %4 = sext i32 %3 to i64
  %5 = getelementptr inbounds float, ptr addrspace(1) %0, i64 %4
  %6 = load float, ptr addrspace(1) %5, align 4, !tbaa !9
  %7 = tail call float @llvm.fmuladd.f32(float %6, float 0x400921FA00000000, float %6)
  %8 = getelementptr inbounds float, ptr addrspace(1) %1, i64 %4
  store float %7, ptr addrspace(1) %8, align 4, !tbaa !9
  ret void
}

; Function Attrs: convergent
declare dso_local i32 @get_global_id(i32 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.fmuladd.f32(float, float, float) #2

attributes #0 = { convergent norecurse nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "uniform-work-group-size"="true" }
attributes #1 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { convergent nounwind }

!llvm.module.flags = !{!0, !1, !2}
!opencl.ocl.version = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, i32 2}
!4 = !{!"clang version 15.0.2"}
!5 = !{i32 0, i32 0}
!6 = !{!"none", !"none"}
!7 = !{!"float*", !"float*"}
!8 = !{!"", !""}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
