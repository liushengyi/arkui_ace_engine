/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
* @Observed class decorator
* 
* usage:
*    @Observed class ClassA { ... }
*
* Causes every instance of decorated clss to be automatically wrapped inside an ObservedObject.
*
* Implemented by extending the decroaetd class by class named 'ObservableObjectClass'.
*
* It is permisstable to decorate the base and the extended class like thisNote: I
*   @Observed class ClassA { ...}
*   @Observed class ClassB extends ClassA { ... }
* and use
*   a = new ClassA();
*   b = new ClassB();
* Only one ES6 Proxy is added.
* 
*
* Take note the decorator implementation extends the prototype chain.
*  
* The prototype chain of a in above example is 
*  - ObservableObjectClass prototype
*  - ClassA prototype
*  - Object prototype
* 
* Snd the prototype chain of b is
*  - ObservableObjectClass prototype
*  - ClassB prototype
*  - ObservableObjectClass prototype
*  - ClassA prototype
*  - Object prototype
*  
* The @Observed decorator is public, part of the SDK, starting from API 9.
*
*/
// define just once to get just one Symbol
const __IS_OBSERVED_PROXIED = Symbol("_____is_observed_proxied__");

function Observed(constructor_: any, _?: any): any {
  stateMgmtConsole.debug(`@Observed class decorator: Overwriting constructor for '${constructor_.name}', gets wrapped inside ObservableObject proxy.`);
  let ObservedClass = class extends constructor_ {
    constructor(...args: any) {
      super(...args);
      stateMgmtConsole.debug(`@Observed '${constructor_.name}' modified constructor.`);
      let isProxied = Reflect.has(this, __IS_OBSERVED_PROXIED);
      Object.defineProperty(this, __IS_OBSERVED_PROXIED, {
        value: true,
        enumerable: false,
        configurable: false,
        writable: false
      });
      if (isProxied) {
        stateMgmtConsole.debug(`   ... new '${constructor_.name}', is proxied already`);
        return this;
      } else {
        stateMgmtConsole.debug(`   ... new '${constructor_.name}', wrapping inside ObservedObject proxy`);
        return ObservedObject.createNewInternal(this, undefined);
      }
    }
  };
  return ObservedClass;
}

// force tsc to generate the __decorate data structure needed for @Observed
// tsc will not generate unless the @Observed class decorator is used at least once
@Observed class __IGNORE_FORCE_decode_GENERATION__ { }


/**
 * class ObservedObject and supporting Handler classes, 
 * Extends from ES6 Proxy. In adding to 'get' and 'set' 
 * the clasess manage subscribers that receive notification
 * about proxies object being 'read' or 'changed'.
 *
 * These classes are framework internal / non-SDK
 *
 */

class SubscribableHandler {
  static readonly SUBSCRIBE = Symbol("_____subscribe__");
  static readonly UNSUBSCRIBE = Symbol("_____unsubscribe__")

  private owningProperties_: Set<number>;

  constructor(owningProperty: IPropertySubscriber) {
    this.owningProperties_ = new Set<number>();
    if (owningProperty) {
      this.addOwningProperty(owningProperty);
    }
    stateMgmtConsole.debug(`SubscribableHandler: constructor done`);
  }

  addOwningProperty(subscriber: IPropertySubscriber): void {
    if (subscriber) {
      stateMgmtConsole.debug(`SubscribableHandler: addOwningProperty: subscriber '${subscriber.id__()}'.`)
      this.owningProperties_.add(subscriber.id__());
    } else {
      stateMgmtConsole.warn(`SubscribableHandler: addOwningProperty: undefined subscriber. - Internal error?`);
    }
  }

  /*
      the inverse function of createOneWaySync or createTwoWaySync
    */
  public removeOwningProperty(property: IPropertySubscriber): void {
    return this.removeOwningPropertyById(property.id__());
  }

  public removeOwningPropertyById(subscriberId: number): void {
    stateMgmtConsole.debug(`SubscribableHandler: removeOwningProperty '${subscriberId}'.`)
    this.owningProperties_.delete(subscriberId);
  }

  protected notifyObjectPropertyHasChanged(propName: string, newValue: any) {
    stateMgmtConsole.debug(`SubscribableHandler: notifyObjectPropertyHasChanged '${propName}'.`)
    this.owningProperties_.forEach((subscribedId) => {
      var owningProperty: IPropertySubscriber = SubscriberManager.Find(subscribedId)

      if (!owningProperty) {
        stateMgmtConsole.warn(`SubscribableHandler: notifyObjectPropertyHasChanged: unknown subscriber.'${subscribedId}' error!.`);
        return;
      }

      // PU code path
      if ('objectPropertyHasChangedPU' in owningProperty) {
        (owningProperty as unknown as ObservedObjectEventsPUReceiver<any>).objectPropertyHasChangedPU(this, propName);
        return;
      }

      // FU code path
      if ('hasChanged' in owningProperty) {
        (owningProperty as ISinglePropertyChangeSubscriber<any>).hasChanged(newValue);
      }
      if ('propertyHasChanged' in owningProperty) {
        (owningProperty as IMultiPropertiesChangeSubscriber).propertyHasChanged(propName);
      }
    });
  }

  // notify a property has been 'read'
  // this functionality is in preparation for observed computed variables
  // enable calling from 'get' trap handler functions to this function once
  // adding support for observed computed variables
  protected notifyObjectPropertyHasBeenRead(propName: string) {
    stateMgmtConsole.debug(`SubscribableHandler: notifyObjectPropertyHasBeenRead '${propName}'.`)
    this.owningProperties_.forEach((subscribedId) => {
      var owningProperty: IPropertySubscriber = SubscriberManager.Find(subscribedId)
      if (owningProperty) {
        // PU code path
        if ('objectPropertyHasBeenReadPU' in owningProperty) {
          (owningProperty as unknown as ObservedObjectEventsPUReceiver<any>).objectPropertyHasBeenReadPU(this, propName);
        }
      }
    });
  }

  public has(target: Object, property: PropertyKey) : boolean {
    stateMgmtConsole.debug(`SubscribableHandler: has '${property.toString()}'.`);
    return (property === ObservedObject.__IS_OBSERVED_OBJECT) ? true : Reflect.has(target, property);
  }

  public get(target: Object, property: PropertyKey, receiver?: any): any {
    stateMgmtConsole.debug(`SubscribableHandler: get '${property.toString()}'.`);
    return (property === ObservedObject.__OBSERVED_OBJECT_RAW_OBJECT) ? target : Reflect.get(target, property, receiver);
  }

  public set(target: Object, property: PropertyKey, newValue: any): boolean {
    switch (property) {
      case SubscribableHandler.SUBSCRIBE:
        // assignment obsObj[SubscribableHandler.SUBSCRCRIBE] = subscriber
        this.addOwningProperty(newValue as IPropertySubscriber);
        return true;
        break;
      case SubscribableHandler.UNSUBSCRIBE:
        // assignment obsObj[SubscribableHandler.UNSUBSCRCRIBE] = subscriber
        this.removeOwningProperty(newValue as IPropertySubscriber);
        return true;
        break;
      default:
        if (Reflect.get(target, property) == newValue) {
          return true;
        }
        stateMgmtConsole.debug(`SubscribableHandler: set '${property.toString()}'.`);
        Reflect.set(target, property, newValue);
        this.notifyObjectPropertyHasChanged(property.toString(), newValue);
        return true;
        break;
    }

    // unreachable
    return false;
  }
}

class SubscribableDateHandler extends SubscribableHandler {

  constructor(owningProperty: IPropertySubscriber) {
    super(owningProperty);
  }

  /**
   * Get trap for Date type proxy
   * Functions that modify Date in-place are intercepted and replaced with a function
   * that executes the original function and notifies the handler of a change.
   * @param target Original Date object
   * @param property
   * @returns
   */
  public get(target: Object, property: PropertyKey): any {

    const dateSetFunctions = new Set(["setFullYear", "setMonth", "setDate", "setHours", "setMinutes", "setSeconds",
      "setMilliseconds", "setTime", "setUTCFullYear", "setUTCMonth", "setUTCDate", "setUTCHours", "setUTCMinutes",
      "setUTCSeconds", "setUTCMilliseconds"]);

    let ret = super.get(target, property);

    if (typeof ret === "function" && property.toString() && dateSetFunctions.has(property.toString())) {
      const self = this;
      return function () {
        // execute original function with given arguments
        let result = ret.apply(this, arguments);
        self.notifyObjectPropertyHasChanged(property.toString(), this);
        return result;
      }.bind(target) // bind "this" to target inside the function
    } else if (typeof ret === "function") {
      ret = ret.bind(target);
    }
    return ret;
  }
}


class ExtendableProxy {
  constructor(obj: Object, handler: SubscribableHandler) {
    return new Proxy(obj, handler);
  }
}

class ObservedObject<T extends Object> extends ExtendableProxy {

  /**
   * Factory function for ObservedObjects /
   *  wrapping of objects for proxying
   *
   * @param rawObject unproxied Object or ObservedObject
   * @param objOwner owner of this Object to sign uop for propertyChange
   *          notifications
   * @returns the rawObject if object is already an ObservedObject,
   *          otherwise the newly created ObservedObject
   */
  public static createNew<T extends Object>(rawObject: T,
    owningProperty: IPropertySubscriber): T {

    if (rawObject === null || rawObject === undefined) {
      stateMgmtConsole.error(`ObservedObject.CreateNew, input object must not be null or undefined.`);
      return rawObject;
    }

    if (ObservedObject.IsObservedObject(rawObject)) {
      ObservedObject.addOwningProperty(rawObject, owningProperty);
      return rawObject;
    }

    return ObservedObject.createNewInternal<T>(rawObject, owningProperty);
  }

  public static createNewInternal<T extends Object>(rawObject: T,
    owningProperty: IPropertySubscriber): T {

    let proxiedObject = new ObservedObject<T>(rawObject,
      Array.isArray(rawObject) ? new class extends SubscribableHandler {
        // In-place array modification functions
        // splice is also in-place modifying function, but we need to handle separately
        private readonly inPlaceModifications: Set<string> = new Set(["copyWithin", "fill", "reverse", "sort"]);

        constructor(owningProperty: IPropertySubscriber) {
          super(owningProperty);
        }

        public get(target: Object, property: PropertyKey, receiver: any): any {
          let ret = super.get(target, property, receiver);
          if (ret && typeof ret === "function") {
            const self = this;
            const prop = property.toString();
            // prop is the function name here
            if (prop == "splice") {
              // 'splice' self modifies the array, returns deleted array items
              // means, alike other self-modifying functions, splice does not return the array itself.
              return function () {
                const result = ret.apply(target, arguments);
                // prop is the function name here
                // and result is the function return value
                // functinon modifies none or more properties
                self.notifyObjectPropertyHasChanged(prop, target);
                return result;
              }.bind(proxiedObject);
            }

            if (self.inPlaceModifications.has(prop)) {
              // in place modfication function result == target, the raw array modified
              stateMgmtConsole.debug("return self mod function");
              return function () {
                const result = ret.apply(target, arguments);

                // 'result' is the unproxied object               
                // functinon modifies none or more properties
                self.notifyObjectPropertyHasChanged(prop, result);

                // returning the 'proxiedObject' ensures that when chain calls also 2nd function call
                // operates on the proxied object.
                return proxiedObject;
              }.bind(proxiedObject);
            }

            // binding the proxiedObject ensures that modifying functions like push() operate on the 
            // proxied array and each array change is notified.
            return ret.bind(proxiedObject);
          }

          return ret;
        }       
      }(owningProperty) // SubscribableArrayHandlerAnonymous
        : (rawObject instanceof Date)
          ? new SubscribableDateHandler(owningProperty)
          : new SubscribableHandler(owningProperty),
      owningProperty);

    return proxiedObject as T;
  }

  /*
    Return the unproxied object 'inside' the ObservedObject / the ES6 Proxy
    no set observation, no notification of changes!
    Use with caution, do not store any references
  */
  static GetRawObject<T extends Object>(obj: T): T {
    return !ObservedObject.IsObservedObject(obj) ? obj : obj[ObservedObject.__OBSERVED_OBJECT_RAW_OBJECT];
  }

  /**
   *
   * @param obj anything
   * @returns true if the parameter is an Object wrpped with a ObservedObject
   * Note: Since ES6 Proying is transparent, 'instance of' will not work. Use
   * this static function instead.
   */
  static IsObservedObject(obj: any): boolean {
    return (obj && (typeof obj === "object") && Reflect.has(obj, ObservedObject.__IS_OBSERVED_OBJECT));
  }

  /**
   * add a subscriber to given ObservedObject
   * due to the proxy nature this static method approach needs to be used instead of a member 
   * function
   * @param obj 
   * @param subscriber 
   * @returns false if given object is not an ObservedObject 
   */
  public static addOwningProperty(obj: Object, subscriber: IPropertySubscriber): boolean {
    if (!ObservedObject.IsObservedObject(obj) || subscriber==undefined) {
      return false;
    }

    obj[SubscribableHandler.SUBSCRIBE] = subscriber;
    return true;
  }

  /**
   * remove a subscriber to given ObservedObject
   * due to the proxy nature this static method approach needs to be used instead of a member 
   * function
   * @param obj 
   * @param subscriber 
   * @returns false if given object is not an ObservedObject 
   */
  public static removeOwningProperty(obj: Object,
    subscriber: IPropertySubscriber): boolean {
    if (!ObservedObject.IsObservedObject(obj)) {
      return false;
    }

    obj[SubscribableHandler.UNSUBSCRIBE] = subscriber;
    return true;
  }

  /**
   * Utility function for debugging the prototype chain of given Object
   * The given object can be any Object, it is not required to be an ObservedObject
   * @param object 
   * @returns multi-line string containing info about the prototype chain
   * on class in class hiararchy per line
   */
  public static tracePrototypeChainOfObject(object: Object | undefined): string {
    let proto = Object.getPrototypeOf(object);
    let result = "";
    let sepa = "";
    while (proto) {
      result += `${sepa}${ObservedObject.tracePrototype(proto)}`;
      proto = Object.getPrototypeOf(proto);
      sepa = ",\n";
    }

    return result;
  }

  /**
   * Utility function for debugging all functions of given Prototype.
   * @returns string containing containing names of all functions and members of given Prototype
   */
  public static tracePrototype(proto: any) {
    if (!proto) {
      return "";
    }

    let result = `${proto.constructor && proto.constructor.name ? proto.constructor.name : '<no class>'}: `;
    let sepa = "";
    for (let name of Object.getOwnPropertyNames(proto)) {
      result += `${sepa}${name}`;
      sepa = ", ";
    };
    return result;
  }

  
  /**
   * @Observed  decorator extends the decorated class. This function returns the prototype of the decorated class
   * @param proto 
   * @returns prototype of the @Observed decorated class or 'proto' parameter if not  @Observed decorated
   */
  public static getPrototypeOfObservedClass(proto: Object): Object {
    return (proto.constructor && proto.constructor.name == "ObservedClass")
      ? Object.getPrototypeOf(proto.constructor.prototype)
      : proto;
  }


  /**
   * To create a new ObservableObject use CreateNew function
   * 
   * constructor create a new ObservableObject and subscribe its owner to propertyHasChanged
   * notifications
   * @param obj  raw Object, if obj is a ObservableOject throws an error
   * @param objectOwner
   */
  private constructor(obj: T, handler: SubscribableHandler, objectOwningProperty: IPropertySubscriber) {
    super(obj, handler);

    if (ObservedObject.IsObservedObject(obj)) {
      stateMgmtConsole.error("ObservableOject constructor: INTERNAL ERROR: after jsObj is observedObject already");
    }
    if (objectOwningProperty != undefined) {
      this[SubscribableHandler.SUBSCRIBE] = objectOwningProperty;
    }
  } // end of constructor

  public static readonly __IS_OBSERVED_OBJECT = Symbol("_____is_observed_object__");
  public static readonly __OBSERVED_OBJECT_RAW_OBJECT = Symbol("_____raw_object__");
}
