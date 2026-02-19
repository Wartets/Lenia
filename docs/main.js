/**
 * Lenia Documentation - Main JavaScript
 * Handles SPA navigation, tab switching, and interactive features
 *
 * @author Colin Bossu Réaubourg (Wartets)
 * @license MIT
 */

(function () {
  "use strict";

  const elements = {
    navLinks: null,
    tabContents: null,
    mobileMenuToggle: null,
    navList: null,
    copyButtons: null,
    canvas: null,
  };

  const state = {
    currentTab: "home",
    isMobileMenuOpen: false,
    isAnimating: false,
  };

  function init() {
    cacheElements();
    setupEventListeners();
    handleInitialRoute();
    initCanvas();
    initMathRendering();
  }

  function cacheElements() {
    elements.navLinks = document.querySelectorAll(".nav-link[data-tab]");
    elements.tabContents = document.querySelectorAll(".tab-content");
    elements.mobileMenuToggle = document.querySelector(".mobile-menu-toggle");
    elements.navList = document.querySelector(".nav-list");
    elements.copyButtons = document.querySelectorAll(".copy-btn");
    elements.canvas = document.getElementById("lenia-placeholder");
  }

  function setupEventListeners() {
    // Navigation tab clicks
    elements.navLinks.forEach((link) => {
      link.addEventListener("click", handleNavClick);
    });

    // Mobile menu toggle
    if (elements.mobileMenuToggle) {
      elements.mobileMenuToggle.addEventListener("click", toggleMobileMenu);
    }

    // Copy buttons
    elements.copyButtons.forEach((btn) => {
      btn.addEventListener("click", handleCopyClick);
    });

    // Tab link buttons within content (e.g., "Getting Started" button)
    document.querySelectorAll("[data-tab-link]").forEach((link) => {
      link.addEventListener("click", handleTabLinkClick);
    });

    // Handle browser back/forward navigation
    window.addEventListener("popstate", handlePopState);

    // Close mobile menu on window resize
    window.addEventListener("resize", debounce(handleResize, 150));

    // Keyboard navigation
    document.addEventListener("keydown", handleKeyDown);
  }

  function handleNavClick(event) {
    event.preventDefault();

    const targetTab = event.currentTarget.getAttribute("data-tab");
    if (targetTab && targetTab !== state.currentTab) {
      switchTab(targetTab, true);
    }

    // Close mobile menu if open
    if (state.isMobileMenuOpen) {
      closeMobileMenu();
    }
  }

  function handleTabLinkClick(event) {
    event.preventDefault();

    const targetTab = event.currentTarget.getAttribute("data-tab-link");
    if (targetTab) {
      switchTab(targetTab, true);
    }
  }

  function switchTab(tabId, updateHistory = false) {
    if (state.isAnimating) return;

    const targetContent = document.getElementById(tabId);
    if (!targetContent) return;

    state.isAnimating = true;
    state.currentTab = tabId;

    // Update navigation active state
    elements.navLinks.forEach((link) => {
      const isActive = link.getAttribute("data-tab") === tabId;
      link.classList.toggle("active", isActive);
      link.setAttribute("aria-current", isActive ? "page" : "false");
    });

    // Update tab content visibility
    elements.tabContents.forEach((content) => {
      const isActive = content.id === tabId;

      if (isActive) {
        content.classList.add("active");
        content.setAttribute("aria-hidden", "false");
      } else {
        content.classList.remove("active");
        content.setAttribute("aria-hidden", "true");
      }
    });

    // Update browser history
    if (updateHistory) {
      const url = tabId === "home" ? window.location.pathname : `#${tabId}`;
      window.history.pushState({ tab: tabId }, "", url);
    }

    // Scroll to top of content
    window.scrollTo({ top: 0, behavior: "smooth" });

    // Re-render math if needed
    if (typeof renderMathInElement === "function") {
      setTimeout(() => {
        renderMathInElement(targetContent, getMathRenderConfig());
      }, 100);
    }

    // Reset animation lock
    setTimeout(() => {
      state.isAnimating = false;
    }, 300);
  }

  function handlePopState(event) {
    const tabId = event.state?.tab || getTabFromHash() || "home";
    switchTab(tabId, false);
  }

  function handleInitialRoute() {
    const initialTab = getTabFromHash() || "home";
    switchTab(initialTab, false);
  }

  function getTabFromHash() {
    const hash = window.location.hash.slice(1);
    const validTabs = ["home", "theory", "features", "installation", "about"];
    return validTabs.includes(hash) ? hash : null;
  }

  function toggleMobileMenu() {
    if (state.isMobileMenuOpen) {
      closeMobileMenu();
    } else {
      openMobileMenu();
    }
  }

  function openMobileMenu() {
    state.isMobileMenuOpen = true;
    elements.navList.classList.add("open");
    elements.mobileMenuToggle.setAttribute("aria-expanded", "true");

    // Animate hamburger to X
    elements.mobileMenuToggle.querySelector(
      ".hamburger",
    ).style.backgroundColor = "transparent";
  }

  function closeMobileMenu() {
    state.isMobileMenuOpen = false;
    elements.navList.classList.remove("open");
    elements.mobileMenuToggle.setAttribute("aria-expanded", "false");

    // Animate X back to hamburger
    elements.mobileMenuToggle.querySelector(
      ".hamburger",
    ).style.backgroundColor = "";
  }

  function handleResize() {
    // Close mobile menu on larger screens
    if (window.innerWidth > 768 && state.isMobileMenuOpen) {
      closeMobileMenu();
    }

    // Resize canvas
    if (elements.canvas) {
      resizeCanvas();
    }
  }

  async function handleCopyClick(event) {
    const button = event.currentTarget;
    const textToCopy = button.getAttribute("data-copy");

    if (!textToCopy) return;

    try {
      await navigator.clipboard.writeText(textToCopy);
      showCopyFeedback(button, true);
    } catch (err) {
      // Fallback for older browsers
      fallbackCopy(textToCopy);
      showCopyFeedback(button, true);
    }
  }

  function fallbackCopy(text) {
    const textarea = document.createElement("textarea");
    textarea.value = text;
    textarea.style.position = "fixed";
    textarea.style.opacity = "0";
    document.body.appendChild(textarea);
    textarea.select();
    document.execCommand("copy");
    document.body.removeChild(textarea);
  }

  function showCopyFeedback(button, success) {
    const originalHTML = button.innerHTML;

    button.innerHTML = success
      ? '<svg viewBox="0 0 24 24" width="16" height="16" fill="currentColor"><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z"/></svg>'
      : '<svg viewBox="0 0 24 24" width="16" height="16" fill="currentColor"><path d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/></svg>';

    button.style.color = success ? "#4ade80" : "#f87171";

    setTimeout(() => {
      button.innerHTML = originalHTML;
      button.style.color = "";
    }, 2000);
  }

  function initCanvas() {
    if (!elements.canvas) return;

    const ctx = elements.canvas.getContext("2d");
    resizeCanvas();

    // Animation state
    const particles = [];
    const particleCount = 50;
    const accentColor = { r: 66, g: 150, b: 250 };

    // Initialize particles
    for (let i = 0; i < particleCount; i++) {
      particles.push({
        x: Math.random() * elements.canvas.width,
        y: Math.random() * elements.canvas.height,
        radius: Math.random() * 3 + 1,
        vx: (Math.random() - 0.5) * 0.5,
        vy: (Math.random() - 0.5) * 0.5,
        alpha: Math.random() * 0.5 + 0.2,
      });
    }

    /**
     * Animation loop
     */
    function animate() {
      ctx.fillStyle = "rgba(10, 10, 12, 0.1)";
      ctx.fillRect(0, 0, elements.canvas.width, elements.canvas.height);

      particles.forEach((particle) => {
        // Update position
        particle.x += particle.vx;
        particle.y += particle.vy;

        // Wrap around edges
        if (particle.x < 0) particle.x = elements.canvas.width;
        if (particle.x > elements.canvas.width) particle.x = 0;
        if (particle.y < 0) particle.y = elements.canvas.height;
        if (particle.y > elements.canvas.height) particle.y = 0;

        // Draw particle
        ctx.beginPath();
        ctx.arc(particle.x, particle.y, particle.radius, 0, Math.PI * 2);
        ctx.fillStyle = `rgba(${accentColor.r}, ${accentColor.g}, ${accentColor.b}, ${particle.alpha})`;
        ctx.fill();
      });

      // Draw connections
      particles.forEach((p1, i) => {
        particles.slice(i + 1).forEach((p2) => {
          const dx = p1.x - p2.x;
          const dy = p1.y - p2.y;
          const distance = Math.sqrt(dx * dx + dy * dy);

          if (distance < 100) {
            ctx.beginPath();
            ctx.moveTo(p1.x, p1.y);
            ctx.lineTo(p2.x, p2.y);
            ctx.strokeStyle = `rgba(${accentColor.r}, ${accentColor.g}, ${accentColor.b}, ${0.1 * (1 - distance / 100)})`;
            ctx.stroke();
          }
        });
      });

      requestAnimationFrame(animate);
    }

    // Check for reduced motion preference
    if (!window.matchMedia("(prefers-reduced-motion: reduce)").matches) {
      animate();
    } else {
      // Static background for reduced motion
      ctx.fillStyle = "#0a0a0c";
      ctx.fillRect(0, 0, elements.canvas.width, elements.canvas.height);
    }
  }

  function resizeCanvas() {
    if (!elements.canvas) return;

    const container = elements.canvas.parentElement;
    const rect = container.getBoundingClientRect();

    elements.canvas.width = rect.width;
    elements.canvas.height = rect.height;
  }

  function initMathRendering() {
    // Wait for KaTeX to load
    if (typeof renderMathInElement === "function") {
      renderAllMath();
    } else {
      document.addEventListener("DOMContentLoaded", () => {
        if (typeof renderMathInElement === "function") {
          renderAllMath();
        }
      });

      // Also try after a short delay for dynamic loading
      setTimeout(() => {
        if (typeof renderMathInElement === "function") {
          renderAllMath();
        }
      }, 500);
    }
  }

  function renderAllMath() {
    if (typeof renderMathInElement !== "function") return;

    document.querySelectorAll(".tab-content").forEach((content) => {
      renderMathInElement(content, getMathRenderConfig());
    });
  }

  function getMathRenderConfig() {
    return {
      delimiters: [
        { left: "$$", right: "$$", display: true },
        { left: "$", right: "$", display: false },
      ],
      throwOnError: false,
      errorColor: "#f87171",
      strict: false,
      trust: true,
      macros: {
        "\\text": "\\textrm",
      },
    };
  }

  function handleKeyDown(event) {
    // Close mobile menu on Escape
    if (event.key === "Escape" && state.isMobileMenuOpen) {
      closeMobileMenu();
      elements.mobileMenuToggle.focus();
    }

    // Navigate tabs with arrow keys when nav is focused
    if (document.activeElement.classList.contains("nav-link")) {
      const links = Array.from(elements.navLinks);
      const currentIndex = links.indexOf(document.activeElement);

      if (event.key === "ArrowRight" || event.key === "ArrowDown") {
        event.preventDefault();
        const nextIndex = (currentIndex + 1) % links.length;
        links[nextIndex].focus();
      } else if (event.key === "ArrowLeft" || event.key === "ArrowUp") {
        event.preventDefault();
        const prevIndex = (currentIndex - 1 + links.length) % links.length;
        links[prevIndex].focus();
      }
    }
  }

  function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
      const later = () => {
        clearTimeout(timeout);
        func(...args);
      };
      clearTimeout(timeout);
      timeout = setTimeout(later, wait);
    };
  }

  function initSmoothScroll() {
    document.querySelectorAll('a[href^="#"]').forEach((anchor) => {
      if (
        !anchor.hasAttribute("data-tab") &&
        !anchor.hasAttribute("data-tab-link")
      ) {
        anchor.addEventListener("click", function (e) {
          const targetId = this.getAttribute("href").slice(1);
          const targetElement = document.getElementById(targetId);

          if (targetElement) {
            e.preventDefault();
            targetElement.scrollIntoView({
              behavior: "smooth",
              block: "start",
            });
          }
        });
      }
    });
  }

  function initScrollAnimations() {
    if (window.matchMedia("(prefers-reduced-motion: reduce)").matches) {
      return;
    }

    const observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            entry.target.classList.add("visible");
            observer.unobserve(entry.target);
          }
        });
      },
      {
        threshold: 0.1,
        rootMargin: "0px 0px -50px 0px",
      },
    );

    // Observe elements that should animate on scroll
    document
      .querySelectorAll(
        ".preview-card, .concept-card, .pipeline-step, .timeline-item",
      )
      .forEach((el) => {
        el.classList.add("fade-in-element");
        observer.observe(el);
      });
  }

  function initVariantCards() {
    const variantCards = document.querySelectorAll(
      ".variant-detailed[onclick]",
    );

    variantCards.forEach((card) => {
      // Add keyboard support for Enter/Space
      card.addEventListener("keydown", (e) => {
        if (e.key === "Enter" || e.key === " ") {
          e.preventDefault();
          card.classList.toggle("expanded");
        }
      });

      // Collapse other cards when one expands (optional - for cleaner UX)
      card.addEventListener("click", () => {
        // Allow multiple expanded cards by not collapsing others
      });
    });
  }

  function initTaxonImages() {
    const taxonImages = document.querySelectorAll(".taxon-image img");

    taxonImages.forEach((img) => {
      // Show placeholder if image fails to load
      img.addEventListener("error", function () {
        this.style.display = "none";
        const placeholder = this.nextElementSibling;
        if (
          placeholder &&
          placeholder.classList.contains("taxon-image-placeholder")
        ) {
          placeholder.style.display = "flex";
        }
      });

      // If image loads successfully, hide placeholder
      img.addEventListener("load", function () {
        const placeholder = this.nextElementSibling;
        if (
          placeholder &&
          placeholder.classList.contains("taxon-image-placeholder")
        ) {
          placeholder.style.display = "none";
        }
      });
    });
  }

  function initColormapPreviews() {
    const previews = document.querySelectorAll(".colormap-preview");

    previews.forEach((preview) => {
      preview.addEventListener("mouseenter", () => {
        preview.style.transform = "scaleY(1.5)";
      });

      preview.addEventListener("mouseleave", () => {
        preview.style.transform = "scaleY(1)";
      });
    });
  }

  function initBackToTop() {
    const backToTopBtn = document.querySelector(".back-to-top");
    const footer = document.querySelector(".main-footer");

    if (!backToTopBtn) return;

    window.addEventListener("scroll", () => {
      if (window.scrollY > 300) {
        backToTopBtn.classList.add("visible");
      } else {
        backToTopBtn.classList.remove("visible");
      }

      if (footer) {
        const footerRect = footer.getBoundingClientRect();
        const viewportHeight = window.innerHeight;
        const btnHeight = 44;
        const btnMargin = 24;
        const defaultBottom = btnMargin;

        if (footerRect.top < viewportHeight) {
          const overlap = viewportHeight - footerRect.top + btnMargin;
          backToTopBtn.style.bottom = overlap + "px";
        } else {
          backToTopBtn.style.bottom = defaultBottom + "px";
        }
      }
    });

    backToTopBtn.addEventListener("click", () => {
      window.scrollTo({
        top: 0,
        behavior: "smooth",
      });
    });
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", () => {
      init();
      initSmoothScroll();
      initScrollAnimations();
      initVariantCards();
      initTaxonImages();
      initColormapPreviews();
      initBackToTop();
      if (window.LeniaI18n) window.LeniaI18n.initTranslations();
    });
  } else {
    init();
    initSmoothScroll();
    initScrollAnimations();
    initVariantCards();
    initTaxonImages();
    initColormapPreviews();
    initBackToTop();
    if (window.LeniaI18n) window.LeniaI18n.initTranslations();
  }
})();
