-keep class com.nvidia.** {*;}
-keep class com.wardrumstudios.** {*;}
-keepclassmembers  class com.nvidia.** {*;}
-keepclassmembers  class com.wardrumstudios.** {*;}
-keep class criminal.russia.GTASA { *; }
-keep class * extends com.wardrumstudios.utils.WarMedia
-keepclasseswithmembernames class * {
     native <methods>;
 }

-keepattributes SourceFile
-keepattributes SourceFile
-renamesourcefileattribute SourceFile
