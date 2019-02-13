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

public class BoolElement : Element {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;

  internal BoolElement(global::System.IntPtr cPtr, bool cMemoryOwn) : base(SharingClientPINVOKE.BoolElement_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(BoolElement obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  ~BoolElement() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          SharingClientPINVOKE.delete_BoolElement(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static BoolElement Cast(Element element) {
    global::System.IntPtr cPtr = SharingClientPINVOKE.BoolElement_Cast(Element.getCPtr(element));
    BoolElement ret = (cPtr == global::System.IntPtr.Zero) ? null : new BoolElement(cPtr, true);
    return ret; 
  }

  public virtual bool GetValue() {
    bool ret = SharingClientPINVOKE.BoolElement_GetValue(swigCPtr);
    return ret;
  }

  public virtual void SetValue(bool newValue) {
    SharingClientPINVOKE.BoolElement_SetValue(swigCPtr, newValue);
  }

}

}
