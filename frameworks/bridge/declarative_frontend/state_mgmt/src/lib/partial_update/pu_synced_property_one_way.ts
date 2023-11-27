/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * SynchedPropertyObjectOneWayPU
 * implementation  of @Prop decorated variables of type class object
 * 
 * all definitions in this file are framework internal
 * 
 */

/**
 * Initialisation scenarios:
 * -------------------------
 * 
 * 1 - no local initialization, source provided (its ObservedObject value)
 *     wrap the ObservedObject into an ObservedPropertyObjectPU
 *     deep copy the ObservedObject into localCopyObservedObject_
 * 
 * 2 - local initialization, no source provided
 *     app transpiled code calls set
 *     leave source_ undefined
 *     no deep copy needed, but provided local init might need wrapping inside an ObservedObject to set to 
 *     localCopyObservedObject_
 * 
 * 3  local initialization,  source provided (its ObservedObject value)
 *    current app transpiled code is not optional
 *    sets source in constructor, as in case 1
 *    calls set() to set the source value, but this will not deepcopy
 * 
 * Update scenarios:
 * -----------------
 * 
 * 1- assignment of a new Object value: this.aProp = new ClassA()
 *    rhs can be ObservedObject because of @Observed decoration or now
 *    notifyPropertyHasChangedPU
 * 
 * 2- local ObservedObject member property change
 *    objectPropertyHasChangedPU called, eventSource is the ObservedObject stored in localCopyObservedObject_
 *    no need to copy, notifyPropertyHasChangedPU
 * 
 * 3- Rerender of the custom component triggered from the parent
 *    reset() is called (code generated by the transpiler), set the value of source_ ,  if that causes a change will call syncPeerHasChanged
 *    syncPeerHasChanged need to deep copy the ObservedObject from source to localCopyObservedObject_
 *    notifyPropertyHasChangedPU
 * 
 * 4- source_ ObservedObject member property change
 *     objectPropertyHasChangedPU called, eventSource is the ObservedObject stored source_.getUnmonitored
 *     notifyPropertyHasChangedPU
 */


class SynchedPropertyOneWayPU<C> extends ObservedPropertyAbstractPU<C>
  implements PeerChangeEventReceiverPU<C>, ObservedObjectEventsPUReceiver<C> {

  // the locally modified ObservedObject
  private localCopyObservedObject_: C;

  // reference to the source variable in parent component
  private source_: ObservedPropertyAbstract<C>;
  // true for @Prop code path, 
  // false for @(Local)StorageProp
  private sourceIsOwnObject : boolean;

  constructor(source: ObservedPropertyAbstract<C> | C,
    owningChildView: IPropertySubscriber,
    thisPropertyName: PropertyInfo) {
    super(owningChildView, thisPropertyName);

    if (source && (typeof (source) === "object") && ("subscribeMe" in source)) {
      // code path for @(Local)StorageProp, the source is a ObservedPropertyObject<C> in a LocalStorage)
      this.source_ = source;
      this.sourceIsOwnObject = false;

      // subscribe to receive value change updates from LocalStorage source property
      this.source_.addSubscriber(this);
    } else {
      const sourceValue = source as C;
      if (this.checkIsSupportedValue(sourceValue)) {
        // code path for 
        // 1- source is of same type C in parent, source is its value, not the backing store ObservedPropertyObject
        // 2- nested Object/Array inside observed another object/array in parent, source is its value
        if (typeof sourceValue == "object" && !((sourceValue instanceof SubscribableAbstract) || ObservedObject.IsObservedObject(sourceValue))) {
          stateMgmtConsole.applicationError(`${this.debugInfo()}:  Provided source object's class is not instance of SubscribableAbstract,
              it also lacks @Observed class decorator. Object property changes will not be observed. Application error!`);
        }
        stateMgmtConsole.debug(`${this.debugInfo()}: constructor: wrapping source in a new ObservedPropertyObjectPU`);
        this.source_ = new ObservedPropertyObjectPU<C>(sourceValue, this, this.getPropSourceObservedPropertyFakeName());
        this.sourceIsOwnObject = true;
      }
    }

    if (this.source_ != undefined) {
      this.resetLocalValue(this.source_.get(), /* needCopyObject */ true);
    }
    stateMgmtConsole.debug(`${this.debugInfo()}: constructor: done!`);
  }


  /*
  like a destructor, need to call this before deleting
  the property.
  */
  aboutToBeDeleted() {
    if (this.source_) {
      this.source_.removeSubscriber(this);
      if (this.sourceIsOwnObject == true && this.source_.numberOfSubscrbers()==0){
        stateMgmtConsole.debug(`${this.debugInfo()}: aboutToBeDeleted. owning source_ ObservedPropertySimplePU, calling its aboutToBeDeleted`);
        this.source_.aboutToBeDeleted();
     }

      this.source_ = undefined;
    }
    super.aboutToBeDeleted();
  }

  public debugInfoDecorator() : string {
    return `@Prop (class SynchedPropertyOneWayPU)`;
  }

  public syncPeerHasChanged(eventSource: ObservedPropertyAbstractPU<C>) {
    stateMgmtProfiler.begin("SyncedPropertyOneWayPU.syncPeerHasChanged");
    if (this.source_ == undefined) {
      stateMgmtConsole.error(`${this.debugInfo()}: syncPeerHasChanged from peer ${eventSource && eventSource.debugInfo && eventSource.debugInfo()}. source_ undefined. Internal error.`);
      stateMgmtProfiler.end();
      return;
    }

    if (eventSource && this.source_ == eventSource) {
      // defensive programming: should always be the case!
      const newValue = this.source_.getUnmonitored();
      if (this.checkIsSupportedValue(newValue)) {
        stateMgmtConsole.debug(`${this.debugInfo()}: syncPeerHasChanged: from peer '${eventSource && eventSource.debugInfo && eventSource.debugInfo()}', local value about to change.`);
        if (this.resetLocalValue(newValue, /* needCopyObject */ true)) {
          this.notifyPropertyHasChangedPU();
        }
      }
    } else {
      stateMgmtConsole.warn(`${this.debugInfo()}: syncPeerHasChanged: from peer '${eventSource?.debugInfo()}', Unexpected situation. syncPeerHasChanged from different sender than source_. Ignoring event.`)
    }
    stateMgmtProfiler.end();
  }

  /**
   * event emited by wrapped ObservedObject, when one of its property values changes
   * @param souceObject 
   * @param changedPropertyName 
   */
  public objectPropertyHasChangedPU(sourceObject: ObservedObject<C>, changedPropertyName: string) {
    stateMgmtConsole.debug(`${this.debugInfo()}: objectPropertyHasChangedPU: property '${changedPropertyName}' of object value has changed.`);
    this.notifyPropertyHasChangedPU();
  }

  public objectPropertyHasBeenReadPU(sourceObject: ObservedObject<C>, changedPropertyName : string) {
    stateMgmtConsole.debug(`${this.debugInfo()}: objectPropertyHasBeenReadPU: property '${changedPropertyName}' of object value has been read.`);
    this.notifyPropertyHasBeenReadPU();
  }

  public getUnmonitored(): C {
    stateMgmtConsole.propertyAccess(`${this.debugInfo()}: getUnmonitored.`);
    // unmonitored get access , no call to notifyPropertyRead !
    return this.localCopyObservedObject_;
  }

  public get(): C {
    stateMgmtConsole.propertyAccess(`${this.debugInfo()}: get.`)
    this.notifyPropertyHasBeenReadPU()
    return this.localCopyObservedObject_;
  }

  // assignment to local variable in the form of this.aProp = <object value>
  // set 'writes through` to the ObservedObject
  public set(newValue: C): void {
    if (this.localCopyObservedObject_ === newValue) {
      stateMgmtConsole.debug(`SynchedPropertyObjectOneWayPU[${this.id__()}IP, '${this.info() || "unknown"}']: set with unchanged value  - nothing to do.`);
      return;
    }

    stateMgmtConsole.propertyAccess(`${this.debugInfo()}: set: value about to change.`);
    if (this.resetLocalValue(newValue, /* needCopyObject */ false)) {
      this.notifyPropertyHasChangedPU();
    }
  }

  // called when updated from parent
  public reset(sourceChangedValue: C): void {
    stateMgmtConsole.propertyAccess(`${this.debugInfo()}: reset (update from parent @Component).`);
    if (this.source_ !== undefined && this.checkIsSupportedValue(sourceChangedValue)) {
      // if this.source_.set causes an actual change, then, ObservedPropertyObject source_ will call syncPeerHasChanged method
      this.source_.set(sourceChangedValue);
    }
  }
  
  /*
    unsubscribe from previous wrapped ObjectObject
    take a shallow or (TODO) deep copy
    copied Object might already be an ObservedObject (e.g. becurse of @Observed decorator) or might be raw
    Therefore, conditionally wrap the object, then subscribe
    return value true only if localCopyObservedObject_ has been changed
  */
    private resetLocalValue(newObservedObjectValue: C, needCopyObject : boolean): boolean {
      // note: We can not test for newObservedObjectValue == this.localCopyObservedObject_
      // here because the object might still be the same, but some property of it has changed
  
      if(!this.checkIsSupportedValue(newObservedObjectValue)) {
        return;
      }
      // unsubscribe from old local copy 
      if (this.localCopyObservedObject_ instanceof SubscribableAbstract) {
        (this.localCopyObservedObject_ as SubscribableAbstract).removeOwningProperty(this);
      } else {
        ObservedObject.removeOwningProperty(this.localCopyObservedObject_, this);
      }

      // shallow/deep copy value 
      // needed whenever newObservedObjectValue comes from source
      // not needed on a local set (aka when called from set() method)
      this.localCopyObservedObject_ = needCopyObject ? this.copyObject(newObservedObjectValue, this.info_) : newObservedObjectValue;

      if (typeof this.localCopyObservedObject_ == "object") {
        if (this.localCopyObservedObject_ instanceof SubscribableAbstract) {
          // deep copy will copy Set of subscribers as well. But local copy only has its own subscribers 
          // not those of its parent value.
          (this.localCopyObservedObject_ as unknown as SubscribableAbstract).clearOwningProperties();
          (this.localCopyObservedObject_ as unknown as SubscribableAbstract).addOwningProperty(this);
        } else if (ObservedObject.IsObservedObject(this.localCopyObservedObject_)) {
          // case: new ObservedObject
          ObservedObject.addOwningProperty(this.localCopyObservedObject_, this);
        } else {
          // wrap newObservedObjectValue raw object as ObservedObject and subscribe to it
          stateMgmtConsole.propertyAccess(`${this.debugInfo()}: Provided source object's is not proxied (is not a ObservedObject). Wrapping it inside ObservedObject.`);
          this.localCopyObservedObject_ = ObservedObject.createNew(this.localCopyObservedObject_, this);
        }
      }
      return true;
  }

  private copyObject(value: C, propName: string): C {
    // ViewStackProcessor.getApiVersion function is not present in API9 
    // therefore shallowCopyObject will always be used in API version 9 and before
    // but the code in this file is the same regardless of API version
    stateMgmtConsole.debug(`${this.debugInfo()}: copyObject: Version: \
    ${(typeof ViewStackProcessor["getApiVersion"] == "function") ? ViewStackProcessor["getApiVersion"]() : 'unknown'}, \
    will use ${((typeof ViewStackProcessor["getApiVersion"] == "function") && (ViewStackProcessor["getApiVersion"]() >= 10)) ? 'deep copy' : 'shallow copy'} .`);

    return ((typeof ViewStackProcessor["getApiVersion"] == "function") &&
      (ViewStackProcessor["getApiVersion"]() >= 10))
      ? this.deepCopyObject(value, propName)
      : this.shallowCopyObject(value, propName);
  }

  // API 9 code path
  private shallowCopyObject(value: C, propName: string): C {
    let rawValue = ObservedObject.GetRawObject(value);
    let copy: C;

    if (!rawValue || typeof rawValue !== 'object') {
      copy = rawValue;
    } else if (typeof rawValue != "object") {
      // FIXME would it be better to throw Exception here?
      stateMgmtConsole.error(`${this.debugInfo()}: shallowCopyObject: request to copy non-object value, actual type is '${typeof rawValue}'. Internal error! Setting copy:=original value.`);
      copy = rawValue;
    } else if (rawValue instanceof Array) {
      // case Array inside ObservedObject
      copy = ObservedObject.createNew([...rawValue] as unknown as C, this);
      Object.setPrototypeOf(copy, Object.getPrototypeOf(rawValue));
    } else if (rawValue instanceof Date) {
      // case Date inside ObservedObject
      let d = new Date();
      d.setTime((rawValue as Date).getTime());
      // subscribe, also Date gets wrapped / proxied by ObservedObject
      copy = ObservedObject.createNew(d as unknown as C, this);
    } else if (rawValue instanceof SubscribableAbstract) {
      // case SubscribableAbstract, no wrapping inside ObservedObject
      copy = { ...rawValue };
      Object.setPrototypeOf(copy, Object.getPrototypeOf(rawValue));
      if (copy instanceof SubscribableAbstract) {
        // subscribe
        (copy as unknown as SubscribableAbstract).addOwningProperty(this);
      }
    } else if (typeof rawValue == "object") {
      // case Object that is not Array, not Date, not SubscribableAbstract
      copy = ObservedObject.createNew({ ...rawValue }, this);
      Object.setPrototypeOf(copy, Object.getPrototypeOf(rawValue));
    } else {
      // TODO in PR "F": change to exception throwing:
      stateMgmtConsole.error(`${this.debugInfo()}: shallow failed. Attempt to copy unsupported value of type '${typeof rawValue}' .`);
      copy = rawValue;
    }

    return copy;
  }

  // API 10 code path
  private deepCopyObject(obj: C, variable?: string): C {
    let copy = SynchedPropertyObjectOneWayPU.deepCopyObjectInternal(obj, variable);

    // this subscribe to the top level object/array of the copy
    // same as shallowCopy does
    if ((obj instanceof SubscribableAbstract) &&
      (copy instanceof SubscribableAbstract)) {
      (copy as unknown as SubscribableAbstract).addOwningProperty(this);
    } else if (ObservedObject.IsObservedObject(obj) && ObservedObject.IsObservedObject(copy)) {
      ObservedObject.addOwningProperty(copy, this);
    }

    return copy;;
  }


  // do not use this function from outside unless it is for testing purposes.
  public static deepCopyObjectInternal<C>(obj: C, variable?: string): C {
    if (!obj || typeof obj !== 'object') {
      return obj;
    }

    let stack = new Array<{ name: string }>();
    let copiedObjects = new Map<Object, Object>();

    return getDeepCopyOfObjectRecursive(obj);
    
    function getDeepCopyOfObjectRecursive(obj: any): any {
      if (!obj || typeof obj !== 'object') {
        return obj;
      }

      const alreadyCopiedObject = copiedObjects.get(obj);
      if (alreadyCopiedObject) {
        let msg = `@Prop deepCopyObject: Found reference to already copied object: Path ${variable ? variable : 'unknown variable'}`;
        stack.forEach(stackItem => msg += ` - ${stackItem.name}`)
        stateMgmtConsole.debug(msg);
        return alreadyCopiedObject;
      }

      let copy;
      if (obj instanceof Set) {
        copy = new Set<any>();
        for (const setKey of obj.keys()) {
          stack.push({ name: setKey });
          copiedObjects.set(obj, copy);
          copy.add(getDeepCopyOfObjectRecursive(setKey));
          stack.pop();
        }
      } else if (obj instanceof Map) {
        copy = new Map<any, any>();
        for (const mapKey of obj.keys()) {
          stack.push({ name: mapKey });
          copiedObjects.set(obj, copy);
          copy.set(mapKey, getDeepCopyOfObjectRecursive(obj.get(mapKey)));
          stack.pop();
        }
      } else if (obj instanceof Date) {
        copy = new Date()
        copy.setTime(obj.getTime());
      } else if (obj instanceof Object) {
        copy = Array.isArray(obj) ? [] : {};
        Object.setPrototypeOf(copy, Object.getPrototypeOf(obj));
        for (const objKey of Object.keys(obj)) {
          stack.push({ name: objKey });
          copiedObjects.set(obj, copy);
          Reflect.set(copy, objKey, getDeepCopyOfObjectRecursive(obj[objKey]));
          stack.pop();
        }
      }
      return ObservedObject.IsObservedObject(obj) ? ObservedObject.createNew(copy, null) : copy;
    }
  }
}

// class definitions for backward compatibility
class SynchedPropertySimpleOneWayPU<T> extends SynchedPropertyOneWayPU<T> {
  
}

class SynchedPropertyObjectOneWayPU<T> extends SynchedPropertyOneWayPU<T> {

}
