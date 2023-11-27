/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
 *
 *  * ViewPU - View for Partial Update
 *
* all definitions in this file are framework internal
*/

/**
 * WeakRef
 * ref to an Object that does not prevent the Object from getting GC'ed
 * current version of tsc does not know about WeakRef
 * but Ark runtime supports it
 *
 */
declare class WeakRef<T extends Object> {
  constructor(o: T);
  deref(): T;
}

declare class DumpLog {
  static print(depth: number, content: string): void;
}

type DFXCommand = { what: string, viewId: number, isRecursive: boolean };
type ProfileRecursionCounter = { total: number };

type ProvidedVarsMapPU = Map<string, ObservedPropertyAbstractPU<any>>;

// denotes a missing elemntId, this is the case during initial render
const UndefinedElmtId = -1;

// function type of partial update function
type UpdateFunc = (elmtId: number, isFirstRender: boolean) => void;

interface UpdateFuncRecord {
  updateFunc: UpdateFunc;
  componentName: string;
  node?: object
}

// function type of recycle node update function
type RecycleUpdateFunc = (elmtId: number, isFirstRender: boolean, recycleNode: ViewPU) => void;

type ExtraInfo = { page:string, line:number };

// NativeView
// implemented in C++  for release
// and in utest/view_native_mock.ts for testing
abstract class ViewPU extends NativeViewPartialUpdate
  implements IViewPropertiesChangeSubscriber {

  // Array.sort() converts array items to string to compare them!
  static readonly compareNumber = (a: number, b: number): number => {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
  };

  private id_: number;

  private parent_: ViewPU = undefined;
  private childrenWeakrefMap_ = new Map<number, WeakRef<ViewPU>>();

  // flag for initgial rendering or re-render on-going.
  private isRenderInProgress: boolean = false;

  // flag if active of inActive
  // inActive means updates are delayed
  private isActive_ : boolean = true;

  private runReuse_: boolean = false;

  private paramsGenerator_: () => Object;

  // flag if {aboutToBeDeletedInternal} is called and the instance of ViewPU has not been GC.
  private isDeleting_: boolean = false;

  private watchedProps: Map<string, (propName: string) => void>
    = new Map<string, (propName: string) => void>();

  private recycleManager: RecycleManager = undefined;

  private extraInfo_:ExtraInfo = undefined;

  // @Provide'd variables by this class and its ancestors
  protected providedVars_: ProvidedVarsMapPU;

  // Set of dependent elmtIds that need partial update
  // during next re-render
  protected dirtDescendantElementIds_: Set<number>
    = new Set<number>();

  // registry of update functions
  // the key is the elementId of the Component/Element that's the result of this function
  protected updateFuncByElmtId: Map<number, UpdateFunc | UpdateFuncRecord>
    = new Map<number, UpdateFunc | UpdateFuncRecord>();

  // set of all @Local/StorageLink/Prop variables owned by this ViwPU
  private ownStorageLinksProps_ : Set<ObservedPropertyAbstractPU<any>> = new Set<ObservedPropertyAbstractPU<any>>();

  // my LocalStorage instance, shared with ancestor Views.
  // create a default instance on demand if none is initialized
  protected localStoragebackStore_: LocalStorage = undefined;

  private ownObservedPropertiesStore__? : Set<ObservedPropertyAbstractPU<any>>;

  private get ownObservedPropertiesStore_() {
    if (!this.ownObservedPropertiesStore__) {
      // lazy init
      this.ownObservedPropertiesStore__ = new Set<ObservedPropertyAbstractPU<any>>();
      this.obtainOwnObservedProperties();
    }
    return this.ownObservedPropertiesStore__;
  }

  protected obtainOwnObservedProperties(): void {
    Object.getOwnPropertyNames(this)
      .filter((propName) => {
        return propName.startsWith("__")
      })
      .forEach((propName) => {
        const stateVar = Reflect.get(this, propName) as Object;
        if ("notifyPropertyHasChangedPU" in stateVar) {
          stateMgmtConsole.debug(`... add state variable ${propName} to ${stateVar}`)
          this.ownObservedPropertiesStore_.add(stateVar as unknown as ObservedPropertyAbstractPU<any>);
        }
      });
  }

  protected get localStorage_() {
    if (!this.localStoragebackStore_ && this.parent_) {
      stateMgmtConsole.debug(`${this.debugInfo()}: constructor: get localStorage_ : Using LocalStorage instance of the parent View.`);
      this.localStoragebackStore_ = this.parent_.localStorage_;
    }

    if (!this.localStoragebackStore_) {
      stateMgmtConsole.info(`${this.debugInfo()}: constructor: is accessing LocalStorage without being provided an instance. Creating a default instance.`);
      this.localStoragebackStore_ = new LocalStorage({ /* empty */ });
    }
    return this.localStoragebackStore_;
  }

  protected set localStorage_(instance: LocalStorage) {
    if (!instance) {
      // setting to undefined not allowed
      return;
    }
    if (this.localStoragebackStore_) {
      stateMgmtConsole.applicationError(`${this.debugInfo()}: constructor: is setting LocalStorage instance twice. Application error.`);
    }
    this.localStoragebackStore_ = instance;
  }

  /**
   * Create a View
   *
   * 1. option: top level View, specify
   *    - compilerAssignedUniqueChildId must specify
   *    - parent=undefined
   *    - localStorage  must provide if @LocalSTorageLink/Prop variables are used
   *      in this View or descendant Views.
   *
   * 2. option: not a top level View
   *    - compilerAssignedUniqueChildId must specify
   *    - parent must specify
   *    - localStorage do not specify, will inherit from parent View.
   *
  */
  constructor(parent: ViewPU, localStorage: LocalStorage, elmtId : number = -1, extraInfo : ExtraInfo = undefined) {
    super();
    // if set use the elmtId also as the ViewPU object's subscribable id.
    // these matching is requiremrnt for updateChildViewById(elmtId) being able to
    // find the child ViewPU object by given elmtId
    this.id_= elmtId == -1 ? SubscriberManager.MakeId() : elmtId;
    this.providedVars_ = parent ? new Map(parent.providedVars_)
      : new Map<string, ObservedPropertyAbstractPU<any>>();

    this.localStoragebackStore_ = undefined;
    stateMgmtConsole.log(`ViewPU constructor: Creating @Component '${this.constructor.name}' from parent '${parent?.constructor.name}}'`);
    if (extraInfo) {
      this.extraInfo_ = extraInfo;
    }
    if (parent) {
      // this View is not a top-level View
      this.setCardId(parent.getCardId());
      // Call below will set this.parent_ to parent as well
      parent.addChild(this);
    } else if (localStorage) {
      this.localStorage_ = localStorage;
      stateMgmtConsole.debug(`${this.debugInfo()}: constructor: Using LocalStorage instance provided via @Entry.`);
    }

    SubscriberManager.Add(this);
    stateMgmtConsole.debug(`${this.debugInfo()}: constructor: done`);
  }

  // globally unique id, this is different from compilerAssignedUniqueChildId!
  id__(): number {
    return this.id_;
  }

  updateId(elmtId: number): void {
    this.id_ = elmtId;
  }

  // inform the subscribed property
  // that the View and thereby all properties
  // are about to be deleted
  abstract aboutToBeDeleted(): void;

  aboutToReuse(params: Object): void {}

  aboutToRecycle(): void {}

  // super class will call this function from
  // its aboutToBeDeleted implementation
  protected aboutToBeDeletedInternal(): void {
    stateMgmtConsole.debug(`${this.debugInfo()}: aboutToBeDeletedInternal`);

    // tell UINodeRegisterProxy that all elmtIds under
    // this ViewPU should be treated as already unregistered

    stateMgmtConsole.debug(`${this.constructor.name}: aboutToBeDeletedInternal `);

    // purge the elementids owning by this viewpu from the updateFuncByElmtId and also the state variable dependent elementids
    Array.from(this.updateFuncByElmtId.keys()).forEach((elemId: number) =>{
      this.purgeDeleteElmtId(elemId);
    })

    if (this.hasRecycleManager()) {
      this.getRecycleManager().purgeAllCachedRecycleNode();
    }

    // unregistration of ElementIDs
    stateMgmtConsole.debug(`${this.debugInfo()}: onUnRegElementID`);

    // it will unregister removed elementids from all the viewpu, equals purgeDeletedElmtIdsRecursively
    this.purgeDeletedElmtIds();
  
    stateMgmtConsole.debug(`${this.debugInfo()}: onUnRegElementID  - DONE`);


    this.updateFuncByElmtId.clear();
    this.watchedProps.clear();
    this.providedVars_.clear();
    this.ownStorageLinksProps_.clear();
    if (this.parent_) {
      this.parent_.removeChild(this);
    }
    this.localStoragebackStore_ = undefined;
    this.isDeleting_ = true;
  }

  public purgeDeleteElmtId(rmElmtId : number ) : boolean {
    stateMgmtConsole.debug(`${this.debugInfo} is purging the rmElmtId:${rmElmtId}`);
    const result = this.updateFuncByElmtId.delete(rmElmtId);
     if (result) {
        this.purgeVariableDependenciesOnElmtIdOwnFunc(rmElmtId);
        // it means rmElmtId has finished all the unregistration from the js side, ElementIdToOwningViewPU_  does not need to keep it
        UINodeRegisterProxy.ElementIdToOwningViewPU_.delete(rmElmtId);
       }
    return result;
  }

  public debugInfo() : string {
    return `@Component '${this.constructor.name}'[${this.id__()}]`;
  }

  // dump info about known elmtIds to a string
  // use function only for debug output and DFX.
  public debugInfoRegisteredElmtIds() : string {
    let result : string = "";
    let sepa : string ="";
    this.updateFuncByElmtId.forEach((value: UpdateFunc | { updateFunc: UpdateFunc; componentName: string; }, elmtId: number) => {
      const compName : string = (typeof value == "object") ? `'${value!.componentName}'` : `'unknown component type'`;
      result += `${sepa}${compName}[${elmtId}]`;
      sepa=", ";
    });
    return result;
  }

  // for given elmtIds look up their component name/type and format a string out of this info
  // use function only for debug output and DFX.
  public debugInfoElmtIds(elmtIds : Array<number>) : string {
    let result : string = "";
    let sepa : string ="";
    elmtIds.forEach((elmtId: number) => {
      result += `${sepa}${this.debugInfoElmtId(elmtId)}`;
      sepa=", ";
    });
    return result;
  }

  public debugInfoElmtId(elmtId : number) : string {
    const updateFuncEntry : UpdateFunc | { updateFunc: UpdateFunc; componentName: string; } = this.updateFuncByElmtId.get(elmtId);
    return (typeof updateFuncEntry == "object") ? `'${updateFuncEntry!.componentName}[${elmtId}]'` : `'unknown component type'[${elmtId}]`;
  }

  public dumpStateVars() : void {
    stateMgmtConsole.debug(`${this.debugInfo()}:  State variables:`);
  }

  private debugInfoStateVars(): string {
    let result: string = `|--${this.constructor.name}[${this.id__()}]`;
    Object.getOwnPropertyNames(this)
      .filter((varName: string) => varName.startsWith("__"))
      .forEach((varName) => {
        const prop: any = Reflect.get(this, varName);
        if ("debugInfoDecorator" in prop) {
          const observedProp = prop as ObservedPropertyAbstractPU<any>;
          result += `\n  ${observedProp.debugInfoDecorator()} '${observedProp.info()}'[${observedProp.id__()}]`;
          result += `\n  ${observedProp.debugInfoSubscribers()}`
          result += `\n  ${observedProp.debugInfoSyncPeers()}`;
          result += `\n  ${observedProp.debugInfoDependentElmtIds()}`
        }
      });
    return result;
  }

  /**
 * ArkUI engine will call this function when the corresponding CustomNode's active status change.
 * @param active true for active, false for inactive
 */
  public setActiveInternal(active: boolean): void {
    stateMgmtProfiler.begin("ViewPU.setActive");
    if (this.isActive_ == active) {
      stateMgmtConsole.debug(`${this.debugInfo()}: setActive ${active} with unchanged state - ignoring`);
      stateMgmtProfiler.end();
      return;
    }
    stateMgmtConsole.debug(`${this.debugInfo()}: setActive ${active ? ' inActive -> active' : 'active -> inActive'}`);
    this.isActive_ = active;
    if (this.isActive_) {
      this.onActiveInternal()
    } else {
      this.onInactiveInternal();
    }
    stateMgmtProfiler.end();
  }

  private onActiveInternal(): void {
    if (!this.isActive_) {
      return;
    }

    stateMgmtConsole.debug(`${this.debugInfo()}: onActiveInternal`);
    this.performDelayedUpdate();
    for (const child of this.childrenWeakrefMap_.values()) {
      const childViewPU: ViewPU | undefined = child.deref();
      if (childViewPU) {
        childViewPU.setActiveInternal(this.isActive_);
      }
    }
  }


  private onInactiveInternal(): void {
    if (this.isActive_) {
      return;
    }

    stateMgmtConsole.debug(`${this.debugInfo()}: onInactiveInternal`);
    for (const storageProp of this.ownStorageLinksProps_) {
      storageProp.enableDelayedNotification();
    }

    for (const child of this.childrenWeakrefMap_.values()) {
      const childViewPU: ViewPU | undefined = child.deref();
      if (childViewPU) {
        childViewPU.setActiveInternal(this.isActive_);
      }
    }
  }

  private setParent(parent: ViewPU) {
    if (this.parent_ && parent) {
      stateMgmtConsole.warn(`${this.debugInfo()}: setChild: changing parent to '${parent?.debugInfo()} (unsafe operation)`);
    }
    this.parent_ = parent;
  }

  /**
   * add given child and set 'this' as its parent
   * @param child child to add
   * @returns returns false if child with given child's id already exists
   *
   * framework internal function
   * Note: Use of WeakRef ensures child and parent do not generate a cycle dependency.
   * The add. Set<ids> is required to reliably tell what children still exist.
   */
  public addChild(child: ViewPU): boolean {
    if (this.childrenWeakrefMap_.has(child.id__())) {
      stateMgmtConsole.warn(`${this.debugInfo()}: addChild '${child?.debugInfo()}' id already exists ${child.id__()}. Internal error!`);
      return false;
    }
    this.childrenWeakrefMap_.set(child.id__(), new WeakRef(child));
    child.setParent(this);
    return true;
  }

  /**
   * remove given child and remove 'this' as its parent
   * @param child child to add
   * @returns returns false if child with given child's id does not exist
   */
  public removeChild(child: ViewPU): boolean {
    const hasBeenDeleted = this.childrenWeakrefMap_.delete(child.id__());
    if (!hasBeenDeleted) {
      stateMgmtConsole.warn(`${this.debugInfo()}: removeChild '${child?.debugInfo()}', child id ${child.id__()} not known. Internal error!`);
    } else {
      child.setParent(undefined);
    }
    return hasBeenDeleted;
  }

  /**
   * Retrieve child by given id
   * @param id
   * @returns child if in map and weak ref can still be downreferenced
   */
  public getChildById(id: number) {
    const childWeakRef = this.childrenWeakrefMap_.get(id);
    return childWeakRef ? childWeakRef.deref() : undefined;
  }

  protected abstract purgeVariableDependenciesOnElmtId(removedElmtId: number);
  protected abstract initialRender(): void;
  protected abstract rerender(): void;
  protected abstract updateRecycleElmtId(oldElmtId: number, newElmtId: number): void;
  protected updateStateVars(params: {}) : void {
    stateMgmtConsole.error(`${this.debugInfo()}: updateStateVars unimplemented. Pls upgrade to latest eDSL transpiler version. Application error.`)
  }

  protected initialRenderView(): void {
    stateMgmtProfiler.begin("ViewPU.initialRenderView");
    this.isRenderInProgress = true;
    this.initialRender();
    this.isRenderInProgress = false;
    stateMgmtProfiler.end();
  }

  private UpdateElement(elmtId: number): void {
    stateMgmtProfiler.begin("ViewPU.UpdateElement");
    if (elmtId == this.id__()) {
      // do not attempt to update itself.
      // a @Prop can add a dependency of the ViewPU onto itself. Ignore it.
      stateMgmtProfiler.end();
      return;
    }

    // do not process an Element that has been marked to be deleted
    const updateFunc1: UpdateFuncRecord | UpdateFunc = this.updateFuncByElmtId.get(elmtId);
    const updateFunc: UpdateFunc | undefined = ((typeof updateFunc1 == "object") ? (updateFunc1!.updateFunc) : updateFunc1) as UpdateFunc | undefined;
    const componentName : string = (typeof updateFunc1 == "object") ? updateFunc1.componentName as string : "unknown component type";

    if (typeof updateFunc !== "function") {
      stateMgmtConsole.debug(`${this.debugInfo()}: update function of elmtId ${elmtId} not found, internal error!`);
    } else {
      stateMgmtConsole.debug(`${this.debugInfo()}: updateDirtyElements: re-render of ${componentName} elmtId ${elmtId} start ...`);
      this.isRenderInProgress = true;
      stateMgmtProfiler.begin("ViewPU.updateFunc");
      updateFunc(elmtId, /* isFirstRender */ false);
      stateMgmtProfiler.end();
      stateMgmtProfiler.begin("ViewPU.finishUpdateFunc (native)");
      this.finishUpdateFunc(elmtId);
      stateMgmtProfiler.end();
      this.isRenderInProgress = false;
      stateMgmtConsole.debug(`${this.debugInfo()}: updateDirtyElements: re-render of ${componentName} elmtId ${elmtId} - DONE`);
    }
    stateMgmtProfiler.end();
  }

  public dumpReport(): void {
      stateMgmtConsole.warn(`Printing profiler information`);
      stateMgmtProfiler.report();
  }
  /**
   * force a complete rerender / update by executing all update functions
   * exec a regular rerender first
   *
   * @param deep recurse all children as well
   *
   * framework internal functions, apps must not call
   */
  public forceCompleteRerender(deep: boolean = false): void {
    stateMgmtProfiler.begin("ViewPU.forceCompleteRerender");
    stateMgmtConsole.warn(`${this.debugInfo()}: forceCompleteRerender - start.`);

    // see which elmtIds are managed by this View
    // and clean up all book keeping for them
    this.purgeDeletedElmtIds();

    Array.from(this.updateFuncByElmtId.keys()).sort(ViewPU.compareNumber).forEach(elmtId => this.UpdateElement(elmtId));

    if (deep) {
      this.childrenWeakrefMap_.forEach((weakRefChild: WeakRef<ViewPU>) => {
        const child = weakRefChild.deref();
        if (child) {
          (child as ViewPU).forceCompleteRerender(true);
        }
      });
    }
    stateMgmtConsole.warn(`${this.debugInfo()}: forceCompleteRerender - end`);
    stateMgmtProfiler.end();
  }

  /**
   * force a complete rerender / update on specific node by executing update function.
   *
   * @param elmtId which node needs to update.
   *
   * framework internal functions, apps must not call
   */
  public forceRerenderNode(elmtId: number): void {
    stateMgmtProfiler.begin("ViewPU.forceRerenderNode");
    // see which elmtIds are managed by this View
    // and clean up all book keeping for them
    this.purgeDeletedElmtIds();
    this.UpdateElement(elmtId);

    // remove elemtId from dirtDescendantElementIds.
    this.dirtDescendantElementIds_.delete(elmtId);
    stateMgmtProfiler.end();
  }

  public updateStateVarsOfChildByElmtId(elmtId, params: Object) : void {
    stateMgmtProfiler.begin("ViewPU.updateStateVarsOfChildByElmtId");
    stateMgmtConsole.debug(`${this.debugInfo()}: updateChildViewById(${elmtId}) - start`);

    if (elmtId<0) {
      stateMgmtConsole.warn(`${this.debugInfo()}: updateChildViewById(${elmtId}) - invalid elmtId - internal error!`);
      stateMgmtProfiler.end();
      return ;
    }
    let child : ViewPU = this.getChildById(elmtId);
    if (!child) {
      stateMgmtConsole.warn(`${this.debugInfo()}: updateChildViewById(${elmtId}) - no child with this elmtId - internal error!`);
      stateMgmtProfiler.end();
      return;
    }
    child.updateStateVars(params);
    stateMgmtConsole.debug(`${this.debugInfo()}: updateChildViewById(${elmtId}) - end`);
    stateMgmtProfiler.end();
  }

  // implements IMultiPropertiesChangeSubscriber
  viewPropertyHasChanged(varName: PropertyInfo, dependentElmtIds: Set<number>): void {
    stateMgmtProfiler.begin("ViewPU.viewPropertyHasChanged");
    stateMgmtTrace.scopedTrace(() => {
      if (this.isRenderInProgress) {
        stateMgmtConsole.applicationError(`${this.debugInfo()}: State variable '${varName}' has changed during render! It's illegal to change @Component state while build (initial render or re-render) is on-going. Application error!`);
      }

      this.syncInstanceId();

      if (dependentElmtIds.size && !this.isFirstRender()) {
        if (!this.dirtDescendantElementIds_.size && !this.runReuse_) {
          // mark ComposedElement dirty when first elmtIds are added
          // do not need to do this every time
          this.markNeedUpdate();
        }
        stateMgmtConsole.debug(`${this.debugInfo()}: viewPropertyHasChanged property: elmtIds that need re-render due to state variable change: ${this.debugInfoElmtIds(Array.from(dependentElmtIds))} .`)
        for (const elmtId of dependentElmtIds) {
          this.dirtDescendantElementIds_.add(elmtId);
        }
        stateMgmtConsole.debug(`   ... updated full list of elmtIds that need re-render [${this.debugInfoElmtIds(Array.from(this.dirtDescendantElementIds_))}].`)
      } else {
        stateMgmtConsole.debug(`${this.debugInfo()}: viewPropertyHasChanged: state variable change adds no elmtIds for re-render`);
        stateMgmtConsole.debug(`   ... unchanged full list of elmtIds that need re-render [${this.debugInfoElmtIds(Array.from(this.dirtDescendantElementIds_))}].`)
      }

      let cb = this.watchedProps.get(varName)
      if (cb) {
        stateMgmtConsole.debug(`   ... calling @Watch function`);
        cb.call(this, varName);
      }

      this.restoreInstanceId();
    }, "ViewPU.viewPropertyHasChanged", this.constructor.name, varName, dependentElmtIds.size);
    stateMgmtProfiler.end();
  }


  private performDelayedUpdate(): void {
    if (!this.ownStorageLinksProps_.size) {
      return;
    }
    stateMgmtProfiler.begin("ViewPU.performDelayedUpdate");
    stateMgmtTrace.scopedTrace(() => {
    stateMgmtConsole.debug(`${this.debugInfo()}: performDelayedUpdate start ...`);
    this.syncInstanceId();

    for (const storageProp of this.ownStorageLinksProps_) {
      const changedElmtIds = storageProp.moveElmtIdsForDelayedUpdate();
      if (changedElmtIds) {
        const varName = storageProp.info();
        if (changedElmtIds.size && !this.isFirstRender()) {
          for (const elmtId of changedElmtIds) {
            this.dirtDescendantElementIds_.add(elmtId);
          }
        }

        stateMgmtConsole.debug(`${this.debugInfo()}: performDelayedUpdate: all elmtIds that need re-render [${Array.from(this.dirtDescendantElementIds_).toString()}].`)

        const cb = this.watchedProps.get(varName)
        if (cb) {
          stateMgmtConsole.debug(`   ... calling @Watch function`);
          cb.call(this, varName);
        }
      }
    } // for all ownStorageLinksProps_
    this.restoreInstanceId();

    if (this.dirtDescendantElementIds_.size) {
      this.markNeedUpdate();
    }

    }, "ViewPU.performDelayedUpdate", this.constructor.name);
    stateMgmtProfiler.end();
  }

  /**
   * Function to be called from the constructor of the sub component
   * to register a @Watch varibale
   * @param propStr name of the variable. Note from @Provide and @Consume this is
   *      the variable name and not the alias!
   * @param callback application defined member function of sub-class
   */
  protected declareWatch(propStr: string, callback: (propName: string) => void): void {
    this.watchedProps.set(propStr, callback);
  }

  /**
   * This View @Provide's a variable under given name
   * Call this function from the constructor of the sub class
   * @param providedPropName either the variable name or the alias defined as
   *        decorator param
   * @param store the backing store object for this variable (not the get/set variable!)
   */
  protected addProvidedVar<T>(providedPropName: string, store: ObservedPropertyAbstractPU<T>) {
    if (this.providedVars_.has(providedPropName)) {
      throw new ReferenceError(`${this.constructor.name}: duplicate @Provide property with name ${providedPropName}.
      Property with this name is provided by one of the ancestor Views already.`);
    }
    this.providedVars_.set(providedPropName, store);
  }

  /**
   * Method for the sub-class to call from its constructor for resolving
   *       a @Consume variable and initializing its backing store
   *       with the SyncedPropertyTwoWay<T> object created from the
   *       @Provide variable's backing store.
   * @param providedPropName the name of the @Provide'd variable.
   *     This is either the @Consume decorator parameter, or variable name.
   * @param consumeVarName the @Consume variable name (not the
   *            @Consume decorator parameter)
   * @returns initializing value of the @Consume backing store
   */
  protected initializeConsume<T>(providedPropName: string,
    consumeVarName: string): ObservedPropertyAbstractPU<T> {
    let providedVarStore : ObservedPropertyAbstractPU<any> = this.providedVars_.get(providedPropName);
    if (providedVarStore === undefined) {
      throw new ReferenceError(`${this.debugInfo()} missing @Provide property with name ${providedPropName}.
          Fail to resolve @Consume(${providedPropName}).`);
    }

    const factory = <T>(source: ObservedPropertyAbstract<T>) => {
      const result : ObservedPropertyAbstractPU<T> = new SynchedPropertyTwoWayPU<T>(source, this, consumeVarName);
      stateMgmtConsole.debug(`The @Consume is instance of ${result.constructor.name}`);
      return result;
    };
    return providedVarStore.createSync(factory) as  ObservedPropertyAbstractPU<T>;
  }


  /**
   * given the elmtId of a child or child of child within this custom component
   * remember this component needs a partial update
   * @param elmtId
   */
  public markElemenDirtyById(elmtId: number): void {
    // TODO ace-ets2bundle, framework, compilated apps need to update together
    // this function will be removed after a short transiition periode
    stateMgmtConsole.applicationError(`${this.debugInfo()}: markElemenDirtyById no longer supported.
        Please update your ace-ets2bundle and recompile your application. Application error!`);
  }

  /**
   * For each recorded dirty Element in this custom component
   * run its update function
   *
   */
  public updateDirtyElements() {
    stateMgmtProfiler.begin("ViewPU.updateDirtyElements");
    do {
        stateMgmtConsole.debug(`${this.debugInfo()}: updateDirtyElements (re-render): sorted dirty elmtIds: ${Array.from(this.dirtDescendantElementIds_).sort(ViewPU.compareNumber)}, starting ....`);

        // see which elmtIds are managed by this View
        // and clean up all book keeping for them
        this.purgeDeletedElmtIds();

        // process all elmtIds marked as needing update in ascending order.
        // ascending order ensures parent nodes will be updated before their children
        // prior cleanup ensure no already deleted Elements have their update func executed
        Array.from(this.dirtDescendantElementIds_).sort(ViewPU.compareNumber).forEach(elmtId => {
            this.UpdateElement(elmtId);
            this.dirtDescendantElementIds_.delete(elmtId);
        });

        if (this.dirtDescendantElementIds_.size) {
          stateMgmtConsole.applicationError(`${this.debugInfo()}: New UINode objects added to update queue while re-render! - Likely caused by @Component state change during build phase, not allowed. Application error!`);
        }
    } while(this.dirtDescendantElementIds_.size);
    stateMgmtConsole.debug(`${this.debugInfo()}: updateDirtyElements (re-render) - DONE, dump of ViewPU in next lines`);
    this.dumpStateVars();
    stateMgmtProfiler.end();
  }

  // request list of all (global) elmtIds of deleted UINodes and unregister from the all viewpus
  // this function equals purgeDeletedElmtIdsRecursively because it does unregistration for all viewpus
  protected purgeDeletedElmtIds(): void {
    stateMgmtConsole.debug(`purgeDeletedElmtIds @Component '${this.constructor.name}' (id: ${this.id__()}) start ...`)
    // request list of all (global) elmtIds of deleted UINodes that need to be unregistered
    UINodeRegisterProxy.obtainDeletedElmtIds();
    // unregister the removed elementids requested from the cpp side for all viewpus, it will make the first viewpu slower
    // than before, but the rest viewpu will be faster
    UINodeRegisterProxy.unregisterElmtIdsFromViewPUs();
    stateMgmtConsole.debug(`purgeDeletedElmtIds @Component '${this.constructor.name}' (id: ${this.id__()}) end... `)
  }


  protected purgeVariableDependenciesOnElmtIdOwnFunc(elmtId: number): void {
    this.ownObservedPropertiesStore_.forEach((stateVar: ObservedPropertyAbstractPU<any>) => {
      stateVar.purgeDependencyOnElmtId(elmtId);
    })
  }

  // executed on first render only
  // kept for backward compatibility with old ace-ets2bundle
  public observeComponentCreation(compilerAssignedUpdateFunc: UpdateFunc): void {
    if (this.isDeleting_) {
      stateMgmtConsole.error(`View ${this.constructor.name} elmtId ${this.id__()} is already in process of destrucion, will not execute observeComponentCreation `);
      return;
    }
    const elmtId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
    stateMgmtConsole.debug(`${this.debugInfo()}: First render for elmtId ${elmtId} start ....`);
    compilerAssignedUpdateFunc(elmtId, /* is first render */ true);
    // in observeComponentCreation function we do not get info about the component name, in 
    // observeComponentCreation2 we do.
    this.updateFuncByElmtId.set(elmtId, { updateFunc: compilerAssignedUpdateFunc, componentName: "unknown" }  );
    // add element id -> owningviewpu
    UINodeRegisterProxy.ElementIdToOwningViewPU_.set(elmtId, new WeakRef(this));
    stateMgmtConsole.debug(`${this.debugInfo()}: First render for elmtId ${elmtId} - DONE.`);
  }

  // executed on first render only
  // added July 2023, replaces observeComponentCreation
  // classObject is the ES6 class object , mandatory to specify even the class lacks the pop function.
  // - prototype : Object is present for every ES6 class
  // - pop : () => void, static function present for JSXXX classes such as Column, TapGesture, etc.
  public observeComponentCreation2(compilerAssignedUpdateFunc: UpdateFunc, classObject: { prototype : Object, pop?: () => void }): void {
    if (this.isDeleting_) {
      stateMgmtConsole.error(`View ${this.constructor.name} elmtId ${this.id__()} is already in process of destrucion, will not execute observeComponentCreation2 `);
      return;
    }
    const _componentName : string =  (classObject && ("name" in classObject)) ? Reflect.get(classObject, "name") as string : "unspecified UINode";
    const _popFunc : () => void = (classObject && "pop" in classObject) ? classObject.pop! : () => {};
    const updateFunc = (elmtId: number, isFirstRender: boolean) => {
      this.syncInstanceId();
      stateMgmtConsole.debug(`${this.debugInfo()}: ${isFirstRender ? `First render` : `Re-render/update`} start ....`);
      ViewStackProcessor.StartGetAccessRecordingFor(elmtId);
      compilerAssignedUpdateFunc(elmtId, isFirstRender);
      if (!isFirstRender) {
        _popFunc();
      }
      ViewStackProcessor.StopGetAccessRecording();
      stateMgmtConsole.debug(`${this.debugInfo()}: ${isFirstRender ? `First render` : `Re-render/update`} - DONE ....`);
      this.restoreInstanceId();
    };

    const elmtId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
    // needs to move set before updateFunc.
    // make sure the key and object value exist since it will add node in attributeModifier during updateFunc.
    this.updateFuncByElmtId.set(elmtId, { updateFunc: updateFunc, componentName: _componentName } );
    // add element id -> owningviewpu
    UINodeRegisterProxy.ElementIdToOwningViewPU_.set(elmtId,  new WeakRef(this));
    try {
      updateFunc(elmtId, /* is first render */ true );
    } catch (error) {
      // avoid the incompatible change that move set function before updateFunc.
      this.updateFuncByElmtId.delete(elmtId);
      UINodeRegisterProxy.ElementIdToOwningViewPU_.delete(elmtId);
      stateMgmtConsole.applicationError(`${this.debugInfo()} has error in update func: ${(error as Error).message}`)
      throw error;
    }
    stateMgmtConsole.debug(`${this.debugInfo()} is initial rendering elmtId ${this.id__()}, tag: ${_componentName}, and updateFuncByElmtId size :${this.updateFuncByElmtId.size}`);
  }

  getOrCreateRecycleManager(): RecycleManager {
    if (!this.recycleManager) {
      this.recycleManager = new RecycleManager
    }
    return this.recycleManager;
  }

  getRecycleManager(): RecycleManager {
    return this.recycleManager;
  }

  hasRecycleManager(): boolean {
    return !(this.recycleManager === undefined);
  }

  initRecycleManager(): void {
    if (this.recycleManager) {
      stateMgmtConsole.error(`${this.debugInfo()}: init recycleManager multiple times. Internal error.`);
      return;
    }
    this.recycleManager = new RecycleManager;
  }

  /**
   * @function observeRecycleComponentCreation
   * @description custom node recycle creation
   * @param name custom node name
   * @param recycleUpdateFunc custom node recycle update which can be converted to a normal update function
   * @return void
   */
  public observeRecycleComponentCreation(name: string, recycleUpdateFunc: RecycleUpdateFunc): void {
    // convert recycle update func to update func
    const compilerAssignedUpdateFunc: UpdateFunc = (element, isFirstRender) => {
      recycleUpdateFunc(element, isFirstRender, undefined)
    };
    let node: ViewPU;
    // if there is no suitable recycle node, run a normal creation function.
    if (!this.hasRecycleManager() || !(node = this.getRecycleManager().popRecycleNode(name))) {
      stateMgmtConsole.debug(`${this.constructor.name}[${this.id__()}]: cannot init node by recycle, crate new node`);
      this.observeComponentCreation(compilerAssignedUpdateFunc);
      return;
    }

    // if there is a suitable recycle node, run a recycle update function.
    const newElmtId: number = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
    const oldElmtId: number = node.id__();
    // store the current id and origin id, used for dirty element sort in {compareNumber}
    recycleUpdateFunc(newElmtId, /* is first render */ true, node);
    const oldEntry: UpdateFunc | UpdateFuncRecord | undefined = this.updateFuncByElmtId.get(oldElmtId);
    this.updateFuncByElmtId.delete(oldElmtId);
    this.updateFuncByElmtId.set(newElmtId, {
      updateFunc: compilerAssignedUpdateFunc,
      componentName: (typeof oldEntry == "object") ? oldEntry.componentName : "unknown",
      node: (typeof oldEntry == "object") ? oldEntry.node : undefined,
    });
    node.updateId(newElmtId);
    node.updateRecycleElmtId(oldElmtId, newElmtId);
    SubscriberManager.UpdateRecycleElmtId(oldElmtId, newElmtId);
  }

  aboutToReuseInternal() {
    this.runReuse_ = true;
    stateMgmtTrace.scopedTrace(() => {
      if (this.paramsGenerator_ && typeof this.paramsGenerator_ == "function") {
        this.aboutToReuse(this.paramsGenerator_());
      }
    }, "aboutToReuse", this.constructor.name);
    this.updateDirtyElements();
    this.childrenWeakrefMap_.forEach((weakRefChild) => {
      const child = weakRefChild.deref();
      if (child) {
        child.aboutToReuseInternal();
      }
    });
    this.runReuse_ = false;
  }

  aboutToRecycleInternal() {
    this.runReuse_ = true;
    stateMgmtTrace.scopedTrace(() => {
      this.aboutToRecycle();
    }, "aboutToRecycle", this.constructor.name);
    this.childrenWeakrefMap_.forEach((weakRefChild) => {
      const child = weakRefChild.deref();
      if (child) {
        child.aboutToRecycleInternal();
      }
    });
    this.runReuse_ = false;
  }

  // add current JS object to it's parent recycle manager
  public recycleSelf(name: string): void {
    if (this.parent_ && !this.parent_.isDeleting_) {
      this.parent_.getOrCreateRecycleManager().pushRecycleNode(name, this);
    } else {
      this.resetRecycleCustomNode();
      stateMgmtConsole.error(`${this.constructor.name}[${this.id__()}]: recycleNode must have a parent`);
    }
  }

  // performs the update on a branch within if() { branch } else if (..) { branch } else { branch }
  public ifElseBranchUpdateFunction(branchId : number, branchfunc : () => void ) : void {
    const oldBranchid : number = If.getBranchId();

    if (branchId == oldBranchid) {
      stateMgmtConsole.debug(`${this.debugInfo()}: ifElseBranchUpdateFunction: IfElse branch unchanged, no work to do.`);
      return;
    }

    // branchid identifies uniquely the if .. <1> .. else if .<2>. else .<3>.branch
    // ifElseNode stores the most recent branch, so we can compare
    // removedChildElmtIds will be filled with the elmtIds of all childten and their children will be deleted in response to if .. else chnage
    let removedChildElmtIds = new Array<number>();
    If.branchId(branchId, removedChildElmtIds);

    // purging these elmtIds from state mgmt will make sure no more update function on any deleted child wi;ll be executed
    stateMgmtConsole.debug(`ViewPU ifElseBranchUpdateFunction: elmtIds need unregister after if/else branch switch: ${JSON.stringify(removedChildElmtIds)}`);
    this.purgeDeletedElmtIds();

    branchfunc();
  }

   /**
    Partial updates for ForEach.
    * @param elmtId ID of element.
    * @param itemArray Array of items for use of itemGenFunc.
    * @param itemGenFunc Item generation function to generate new elements. If index parameter is
    *                    given set itemGenFuncUsesIndex to true.
    * @param idGenFunc   ID generation function to generate unique ID for each element. If index parameter is
    *                    given set idGenFuncUsesIndex to true.
    * @param itemGenFuncUsesIndex itemGenFunc optional index parameter is given or not.
    * @param idGenFuncUsesIndex idGenFunc optional index parameter is given or not.
    */
  public forEachUpdateFunction(elmtId : number,
    itemArray: Array<any>,
    itemGenFunc: (item: any, index?: number) => void,
    idGenFunc?: (item: any, index?: number) => string,
    itemGenFuncUsesIndex: boolean = false,
    idGenFuncUsesIndex: boolean = false) : void {

    stateMgmtProfiler.begin("ViewPU.forEachUpdateFunction");
    stateMgmtConsole.debug(`${this.debugInfo()}: forEachUpdateFunction (ForEach re-render) start ...`);

    if (itemArray === null || itemArray === undefined) {
      stateMgmtConsole.applicationError(`${this.debugInfo()}: forEachUpdateFunction (ForEach re-render): input array is null or undefined error. Application error!`);
      stateMgmtProfiler.end();
      return;
    }

    if (itemGenFunc === null || itemGenFunc === undefined) {
      stateMgmtConsole.applicationError(`${this.debugInfo()}: forEachUpdateFunction (ForEach re-render): Item generation function missing. Application error!`);
      stateMgmtProfiler.end();
      return;
    }

    if (idGenFunc === undefined) {
      stateMgmtConsole.debug(`${this.debugInfo()}: forEachUpdateFunction: providing default id gen function `);
      idGenFuncUsesIndex = true;
      // catch possible error caused by Stringify and re-throw an Error with a meaningful (!) error message
      idGenFunc = (item: any, index : number) => {
        try {
          return `${index}__${JSON.stringify(item)}`;
        } catch(e) {
          throw new Error (`${this.debugInfo()}: ForEach id ${elmtId}: use of default id generator function not possible on provided data structure. Need to specify id generator function (ForEach 3rd parameter). Application Error!`)
        }
      }
    }

    let diffIndexArray = []; // New indexes compared to old one.
    let newIdArray = [];
    let idDuplicates = [];
    const arr = itemArray; // just to trigger a 'get' onto the array

    // ID gen is with index.
    if (idGenFuncUsesIndex) {
      // Create array of new ids.
      arr.forEach((item, indx) => {
        newIdArray.push(idGenFunc(item, indx));
      });
    }
    else {
      // Create array of new ids.
      arr.forEach((item, index) => {
        newIdArray.push(`${itemGenFuncUsesIndex ? index + '_':''}` + idGenFunc(item));
      });
    }

    // Set new array on C++ side.
    // C++ returns array of indexes of newly added array items.
    // these are indexes in new child list.
    ForEach.setIdArray(elmtId, newIdArray, diffIndexArray, idDuplicates);

    // Its error if there are duplicate IDs.
    if (idDuplicates.length > 0) {
      idDuplicates.forEach((indx) => {
        stateMgmtConsole.error(`Error: ${newIdArray[indx]} generated for ${indx}${indx < 4 ? indx == 2 ? "nd" : "rd" : "th"} array item ${arr[indx]}.`);
      });
      stateMgmtConsole.applicationError(`${this.debugInfo()}: Ids generated by the ForEach id gen function must be unique. Application error!`);
    }

    stateMgmtConsole.debug(`${this.debugInfo()}: forEachUpdateFunction: diff indexes ${JSON.stringify(diffIndexArray)} . `);

    // Item gen is with index.
    stateMgmtConsole.debug(`   ... item Gen ${itemGenFuncUsesIndex ? 'with' : "without"} index`);
    // Create new elements if any.
    stateMgmtProfiler.begin("ViewPU.forEachUpdateFunction (native)");
    diffIndexArray.forEach((indx) => {
      ForEach.createNewChildStart(newIdArray[indx], this);
      if (itemGenFuncUsesIndex) {
        itemGenFunc(arr[indx], indx);
      } else {
        itemGenFunc(arr[indx]);
      }
      ForEach.createNewChildFinish(newIdArray[indx], this);
    });
    stateMgmtConsole.debug(`${this.debugInfo()}: forEachUpdateFunction (ForEach re-render) - DONE.`);
    stateMgmtProfiler.end();
    stateMgmtProfiler.end();
  }

  /**
     * CreateStorageLink and CreateStorageLinkPU are used by the implementation of @StorageLink and
     * @LocalStotrageLink in full update and partial update solution respectively.
     * These are not part of the public AppStorage API , apps should not use.
     * @param storagePropName - key in LocalStorage
     * @param defaultValue - value to use when creating a new prop in the LocalStotage
     * @param owningView - the View/ViewPU owning the @StorageLink/@LocalStorageLink variable
     * @param viewVariableName -  @StorageLink/@LocalStorageLink variable name
     * @returns SynchedPropertySimple/ObjectTwoWay/PU
     */
  public createStorageLink<T>(storagePropName: string, defaultValue: T, viewVariableName: string): ObservedPropertyAbstractPU<T> {
    const appStorageLink = AppStorage.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyTwoWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    this.ownStorageLinksProps_.add(appStorageLink);
    return appStorageLink;
  }

  public createStorageProp<T>(storagePropName: string, defaultValue: T, viewVariableName: string): ObservedPropertyAbstractPU<T> {
    const appStorageProp = AppStorage.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyOneWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    this.ownStorageLinksProps_.add(appStorageProp);
    return appStorageProp;
  }

  public createLocalStorageLink<T>(storagePropName: string, defaultValue: T,
    viewVariableName: string): ObservedPropertyAbstractPU<T> {
      const localStorageLink =  this.localStorage_.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyTwoWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    this.ownStorageLinksProps_.add(localStorageLink);
    return localStorageLink;
}

  public createLocalStorageProp<T>(storagePropName: string, defaultValue: T,
    viewVariableName: string): ObservedPropertyAbstractPU<T> {
      const localStorageProp = this.localStorage_.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyObjectOneWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    this.ownStorageLinksProps_.add(localStorageProp);
    return localStorageProp;
  }

  public createOrGetNode(elmtId: number, builder: () => object): object {
    const entry = this.updateFuncByElmtId.get(elmtId);
    if (entry === undefined) {
      throw new Error(`${this.debugInfo()} fail to create node, elemtId is illegal`);
    }
    if (typeof entry !== 'object') {
      throw new Error('need update toolchain version');
    }
    let nodeInfo = (entry as UpdateFuncRecord).node;
    if (nodeInfo === undefined) {
      nodeInfo = builder();
      entry.node = nodeInfo;
    }
    return nodeInfo;
  }

  /**
   * onDumpInfo is used to process commands delivered by the hidumper process
   * @param commands -  list of commands provided in the shell
   * @returns void
   */
  protected onDumpInfo(commands: string[]): void {

    let dfxCommands: DFXCommand[] = this.processOnDumpCommands(commands);

    dfxCommands.forEach((command) => {
      let view: ViewPU = undefined;
      if(command.viewId) {
        view = this.findViewInHierarchy(command.viewId);
        if(!view){
          DumpLog.print(0, `\nTarget view: ${command.viewId} not found for command: ${command.what}\n`);
          return;
        }
      } else {
        view = this;
        command.viewId = view.id__();
      }
      switch(command.what) {
        case "-dumpAll":
          view.printDFXHeader("ViewPU Info", command);
          DumpLog.print(0, view.debugInfoView(command.isRecursive));
          break;
        case "-viewHierarchy":
          view.printDFXHeader("ViewPU Hierarchy", command);
          DumpLog.print(0, view.debugInfoViewHierarchy(command.isRecursive));
          break;
        case "-stateVariables":
          view.printDFXHeader("ViewPU State Variables", command);
          DumpLog.print(0, view.debugInfoStateVars());
          break;
        case "-registeredElementIds":
          view.printDFXHeader("ViewPU Registered Element IDs", command);
          DumpLog.print(0, view.debugInfoUpdateFuncByElmtId(command.isRecursive));
          break;
        case "-dirtyElementIds":
          view.printDFXHeader("ViewPU Dirty Registered Element IDs", command);
          DumpLog.print(0, view.debugInfoDirtDescendantElementIds(command.isRecursive));
          break;
        case "-profiler":
          view.printDFXHeader("Profiler Info", command);
          view.dumpReport();
          break;
        default:
          DumpLog.print(0, `\nUnsupported JS DFX dump command: [${command.what}, viewId=${command.viewId}, isRecursive=${command.isRecursive}]\n`);
    }
    })
  }

  private printDFXHeader(header: string, command: DFXCommand): void {
    let length:number = 50;
    let remainder: number = length - header.length < 0 ? 0 : length - header.length;
    DumpLog.print(0, `\n${'-'.repeat(remainder/2)}${header}${'-'.repeat(remainder/2)}`);
    DumpLog.print(0, `[${command.what}, viewId=${command.viewId}, isRecursive=${command.isRecursive}]\n`);
  }

  private processOnDumpCommands(commands: string[]): DFXCommand[] {
    let isFlag: Function = (param: string): boolean => {
      return "-r".match(param) != null || param.startsWith("-viewId=");
    }

    let dfxCommands: DFXCommand[] = [];

    for(var i: number = 0; i < commands.length; i++) {
      let command = commands[i];
      if(isFlag(command)) {
        if(command.startsWith("-viewId=")){
          let dfxCommand:DFXCommand = dfxCommands[dfxCommands.length - 1];
          if(dfxCommand) {
            let input: string[] = command.split('=');
            if(input[1]) {
              let viewId: number = Number.parseInt(input[1]);
              dfxCommand.viewId = Number.isNaN(viewId) ? -1 : viewId; 
            }
          }
        } else if(command.match("-r")){
          let dfxCommand:DFXCommand = dfxCommands[dfxCommands.length - 1];
          if(dfxCommand) {
            dfxCommand.isRecursive = true;
          }
        } 
      } else {
          dfxCommands.push({
            what: command,
            viewId: undefined,
            isRecursive: false,  
          })
      }
    }
    return dfxCommands;
  }

  private findViewInHierarchy(id: number): ViewPU {
    let weak = this.childrenWeakrefMap_.get(id);
    if(weak) {
      return weak.deref();
    }

    let retVal: ViewPU = undefined;
    for(const [key, value] of this.childrenWeakrefMap_.entries()){
        retVal = value.deref().findViewInHierarchy(id);
        if(retVal)
          break;
    }
    return retVal;
  }

  private debugInfoView(recursive: boolean = false): string {
    return this.debugInfoViewInternal(recursive);
  }

  private debugInfoViewInternal(recursive: boolean = false): string {
    let retVal:string = `@Component\n${this.constructor.name}[${this.id__()}]`;
    retVal += `\n\nView Hierarchy:\n${this.debugInfoViewHierarchy(recursive)}`;
    retVal += `\n\nState variables:\n${this.debugInfoStateVars()}`;
    retVal += `\n\nRegistered Element IDs:\n${this.debugInfoUpdateFuncByElmtId(recursive)}`;
    retVal += `\n\nDirty Registered Element IDs:\n${this.debugInfoDirtDescendantElementIds(recursive)}`;
    return retVal;
  }

  private debugInfoViewHierarchy(recursive: boolean = false): string {
    return this.debugInfoViewHierarchyInternal(0, recursive);
  }

  private debugInfoViewHierarchyInternal(depth: number = 0, recursive: boolean = false): string {
    let retVaL: string = `\n${"  ".repeat(depth)}|--${this.constructor.name}[${this.id__()}]`;
    if(depth < 1 || recursive){
      this.childrenWeakrefMap_.forEach((value, key, map) => {
        retVaL += value.deref()?.debugInfoViewHierarchyInternal(depth + 1, recursive);
      }) 
    }
    return retVaL;
  }
  
  private debugInfoUpdateFuncByElmtId(recursive: boolean = false): string {
    return this.debugInfoUpdateFuncByElmtIdInternal({total: 0}, 0, recursive);
  }

  private debugInfoUpdateFuncByElmtIdInternal(counter: ProfileRecursionCounter, depth: number = 0, recursive: boolean = false): string {
    let retVaL: string = `\n${"  ".repeat(depth)}|--${this.constructor.name}[${this.id__()}]: {`;
    this.updateFuncByElmtId.forEach((value, key, map) => {
      retVaL += `\n${"  ".repeat(depth + 2)}${(typeof value == "object") ? value.componentName : "unknown component"}[${key}]`
    })
    counter.total += this.updateFuncByElmtId.size;
    retVaL += `\n${"  ".repeat(depth + 1)}}[${this.updateFuncByElmtId.size}]`
    if(recursive){
      this.childrenWeakrefMap_.forEach((value, key, map) => {
        retVaL += value.deref()?.debugInfoUpdateFuncByElmtIdInternal(counter, depth + 1, recursive);
      })
    }
    if(recursive && depth == 0) {
      retVaL += `\nTotal: ${counter.total}`
    }
    return retVaL;
  }

  private debugInfoDirtDescendantElementIds(recursive: boolean = false): string {
    return this.debugInfoDirtDescendantElementIdsInternal(0, recursive, {total: 0});
  }

  private debugInfoDirtDescendantElementIdsInternal(depth: number = 0, recursive: boolean = false, counter: ProfileRecursionCounter): string {
    let retVaL: string = `\n${"  ".repeat(depth)}|--${this.constructor.name}[${this.id__()}]: {`;
    this.dirtDescendantElementIds_.forEach((value) => {
      retVaL += `${value}, `
    })
    counter.total += this.dirtDescendantElementIds_.size;
    retVaL += `\n${"  ".repeat(depth + 1)}}[${this.dirtDescendantElementIds_.size}]`
    if(recursive){
      this.childrenWeakrefMap_.forEach((value, key, map) => {
        retVaL += value.deref()?.debugInfoDirtDescendantElementIdsInternal(depth + 1, recursive, counter);
      })
    }

    if(recursive && depth == 0){
      retVaL += `\nTotal: ${counter.total}`
    }
    return retVaL;
  }

}
