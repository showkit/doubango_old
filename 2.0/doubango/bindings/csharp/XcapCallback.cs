/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.2
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace org.doubango.tinyWRAP {

using System;
using System.Runtime.InteropServices;

public class XcapCallback : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal XcapCallback(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(XcapCallback obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~XcapCallback() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          tinyWRAPPINVOKE.delete_XcapCallback(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public XcapCallback() : this(tinyWRAPPINVOKE.new_XcapCallback(), true) {
    SwigDirectorConnect();
  }

  public virtual int onEvent(XcapEvent e) {
    int ret = (SwigDerivedClassHasMethod("onEvent", swigMethodTypes0) ? tinyWRAPPINVOKE.XcapCallback_onEventSwigExplicitXcapCallback(swigCPtr, XcapEvent.getCPtr(e)) : tinyWRAPPINVOKE.XcapCallback_onEvent(swigCPtr, XcapEvent.getCPtr(e)));
    return ret;
  }

  private void SwigDirectorConnect() {
    if (SwigDerivedClassHasMethod("onEvent", swigMethodTypes0))
      swigDelegate0 = new SwigDelegateXcapCallback_0(SwigDirectoronEvent);
    tinyWRAPPINVOKE.XcapCallback_director_connect(swigCPtr, swigDelegate0);
  }

  private bool SwigDerivedClassHasMethod(string methodName, Type[] methodTypes) {
    System.Reflection.MethodInfo methodInfo = this.GetType().GetMethod(methodName, System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance, null, methodTypes, null);
    bool hasDerivedMethod = methodInfo.DeclaringType.IsSubclassOf(typeof(XcapCallback));
    return hasDerivedMethod;
  }

  private int SwigDirectoronEvent(IntPtr e) {
    return onEvent((e == IntPtr.Zero) ? null : new XcapEvent(e, false));
  }

  public delegate int SwigDelegateXcapCallback_0(IntPtr e);

  private SwigDelegateXcapCallback_0 swigDelegate0;

  private static Type[] swigMethodTypes0 = new Type[] { typeof(XcapEvent) };
}

}
