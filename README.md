# Sleep background plugin

This plugin allows you to sleep in HSL (```sleep-background(seconds)```) while suspending the thread execution. The only garantee is that the thread will sleep at least x seconds before the thread is re-scheduled.

## Installation

Follow the [instructions](https://docs.halon.io/manual/comp_install.html#installation) in our manual to add our package repository and then run the below command.

### Ubuntu

```
apt-get install halon-extras-sleep-background
```

### RHEL

```
yum install halon-extras-sleep-background
```

## Exported functions

These functions needs to be [imported](https://docs.halon.io/hsl/structures.html#import) from the `extras://sleep-background` module path.

### sleep_background(seconds)

Sleep at least x seconds.

**Params**

- seconds `number` - Time to sleep in seconds

**Returns**

Returns the actual time slept.

**Example**

```
import { sleep_background } from "extras://sleep-background";
echo sleep_background(3.0);
```