import { useEffect, useState } from "react";

import { ProvidersProps } from "@/app/providers";

export function ClientOnly({ children, ...delegated }: ProvidersProps) {
  const [hasMounted, setHasMounted] = useState(false);

  useEffect(() => {
    setHasMounted(true);
  }, []);

  if (!hasMounted) {
    return null;
  }

  return <div {...delegated}>{children}</div>;
}
