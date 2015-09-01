/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.vivantecorp.graphics.gl11tutorial1;

// Wrapper for native library

public class GL11JNILib {

     static {
         System.loadLibrary("gl11tutorial1_jni");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native boolean init(int width, int height);
     public static native boolean repaint();
     public static native boolean changeBackground();
	 public static native boolean key(int key, boolean down);
}
