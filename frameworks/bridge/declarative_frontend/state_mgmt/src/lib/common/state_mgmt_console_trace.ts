/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
 * state mgmt library uses its own class for logging
* allows to remap separately from other use of aceConsole
*
* everything in this file is framework internal
*/

class stateMgmtConsole {
    public static log (...args : any) : void {
        aceConsole.log(...args);
    }
    public static debug(...args : any) : void {
        aceConsole.debug(...args);
    }
    public static info (...args : any) : void {
        aceConsole.info(...args);
    }
    public static warn (...args : any) : void  {
        aceConsole.warn(...args);
    }
    public static error (...args : any) : void {
        aceConsole.error(...args);
    }
    public static propertyAccess (...args : any) : void {
        // enable for fran gran debugging variables observation
        // this code line has been left in intentionally
        // aceConsole.debug(...args);
    }
    public static applicationError (...args : any) : void {
        aceConsole.error(`FIX THIS APPLICATION ERROR \n`, ...args);
    }
}

type TraceArgs = string | number | boolean;

class stateMgmtTrace {
    public static scopedTrace<T>(codeBlock: () => T, arg1: string, ...args: TraceArgs[]): T{
        aceTrace.begin(arg1, ...args);
        let result: T = codeBlock();
        aceTrace.end();
        return result;
    }
}

class errorReport {
    public static varValueCheckFailed<T>(params : { customComponent : string, variableDeco: string, variableName: string, expectedType: string, value : T }) : void {
        let msg = `@Component '${params.customComponent}': Illegal variable value error with decorated variable ${params.variableDeco} '${params.variableName}': `;
        msg += `failed validation: '${params.expectedType}`;
        try {
            msg += `, attempt to assign value type: '${typeof params.value}'`;
            msg += `, value: '${JSON.stringify(params.value, null, 4)}'`;
        } catch(e) {}
        
        msg += "!";
        throw new TypeError(msg);
    }

    public static varObservationFailed<T>(params : { customComponent : string, variableDeco: string, variableName: string, value: T  }) : void {
        let msg = `@Component '${params.customComponent}': decorated variable ${params.variableDeco} '${params.variableName}': `;
        msg += `its class is neither decorated with '@Observed' nor it is an instance of 'SubscribableAbstract'`;

        try {
            msg += `, attempt to assign value type: '${typeof params.value}'`;
            msg += `, value: '${JSON.stringify(params.value, null, 4)}'`;
        } catch(e) {}
        
        msg += "!";

        throw new TypeError(msg);
    }
}