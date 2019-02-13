//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (http://www.swig.org).
// Version 3.0.10
//
// Do not make changes to this file unless you know what you are doing--modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------

namespace MixedRealityToolkit.Sharing {

public class LongElement : Element {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;

  internal LongElement(global::System.IntPtr cPtr, bool cMemoryOwn) : base(SharingClientPINVOKE.LongElement_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(LongElement obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  ~LongElement() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          SharingClientPINVOKE.delete_LongElement(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static LongElement Cast(Element element) {
    global::System.IntPtr cPtr = SharingClientPINVOKE.LongElement_Cast(Element.getCPtr(element));
    LongElement ret = (cPtr == global::System.IntPtr.Zero) ? null : new LongElement(cPtr, true);
    return ret; 
  }

  public virtual long GetValue() {
    long ret = SharingClientPINVOKE.LongElement_GetValue(swigCPtr);
    return ret;
  }

  public virtual void SetValue(long newValue) {
    SharingClientPINVOKE.LongElement_SetValue(swigCPtr, newValue);
  }

}

}
