"use client";

import {
  Navbar as HeroUINavbar,
  NavbarContent,
  NavbarMenuToggle,
  NavbarBrand,
  NavbarItem,
  NavbarMenu,
  NavbarMenuItem,
} from "@heroui/navbar";
import { Button } from "@heroui/button";
import { Link } from "@heroui/link";
import { usePathname } from "next/navigation";
import { useState } from "react";
import { useAtomValue } from "jotai";

import { siteConfig } from "@/config/site";
import { ThemeSwitch } from "@/components/theme-switch";
import { GithubIcon, HeartFilledIcon, Logo } from "@/components/icons";
import { BACKEND_URL_OK } from "@/config/env";
import { ServerOff } from "lucide-react";

export const Navbar = () => {
  const pathname = usePathname();
  const [isMenuOpen, setIsMenuOpen] = useState(false);
  const backendUrlOk = useAtomValue(BACKEND_URL_OK);

  return (
    <HeroUINavbar
      isBordered
      isMenuOpen={isMenuOpen}
      maxWidth="xl"
      onMenuOpenChange={setIsMenuOpen}
    >
      <NavbarContent className="basis-1/5 sm:basis-full" justify="start">
        <NavbarBrand as="li" className="gap-3 max-w-fit">
          <Link
            className="flex justify-start items-center gap-1"
            color="foreground"
            href="/"
          >
            <Logo />
            <p className="font-bold text-inherit">DanMuCrawl</p>
          </Link>
        </NavbarBrand>
        <ul className="hidden sm:flex gap-4 justify-start ml-2">
          {siteConfig.navItems.map((item) => {
            const linkBad = !!(item.backendRequired && backendUrlOk);

            return (
              <NavbarItem key={item.href}>
                <Link
                  color={item.href === pathname ? "secondary" : "foreground"}
                  href={item.href}
                  isDisabled={linkBad}
                >
                  {linkBad && <ServerOff size={15} />}
                  {item.label}
                </Link>
              </NavbarItem>
            );
          })}
        </ul>
      </NavbarContent>

      <NavbarContent
        className="hidden sm:flex basis-1/5 sm:basis-full"
        justify="end"
      >
        <NavbarItem className="hidden sm:flex gap-2">
          <Link isExternal aria-label="Github" href={siteConfig.links.github}>
            <GithubIcon className="text-default-500" />
          </Link>
          <ThemeSwitch />
        </NavbarItem>
        <NavbarItem className="hidden md:flex">
          <Button
            isExternal
            as={Link}
            className="text-sm font-normal text-default-600 bg-default-100"
            href={siteConfig.links.sponsor}
            startContent={<HeartFilledIcon className="text-danger" />}
            variant="flat"
          >
            作者
          </Button>
        </NavbarItem>
      </NavbarContent>

      <NavbarContent className="sm:hidden basis-1 pl-4" justify="end">
        <Link isExternal aria-label="Github" href={siteConfig.links.github}>
          <GithubIcon className="text-default-500" />
        </Link>
        <ThemeSwitch />
        <NavbarMenuToggle />
      </NavbarContent>

      <NavbarMenu>
        {siteConfig.navItems.map((item) => {
          const linkBad = !!(item.backendRequired && backendUrlOk);

          return (
            <NavbarMenuItem key={item.href}>
              <Link
                className="w-full"
                color={item.href === pathname ? "secondary" : "foreground"}
                href={item.href}
                isDisabled={linkBad}
                onPressEnd={() => setIsMenuOpen(false)}
              >
                {linkBad && <ServerOff size={15} />}
                {item.label}
              </Link>
            </NavbarMenuItem>
          );
        })}
      </NavbarMenu>
    </HeroUINavbar>
  );
};
